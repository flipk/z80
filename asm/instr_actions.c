
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "instr_table.h"
#include "instr_actions.h"

static void print_set( int offset, struct instruction_type * );
static void print_instr( int opcode, struct instruction_type * in );
static void print_op( int type, int op );
static char * regnum_to_reg( int reg );
static char * flagnum_to_reg( int flag );

static struct compiled_instruction * instructions;
static struct compiled_instruction ** nist;

struct compiled_instruction cur;

static void
_compile( int start, struct instruction_type * it )
{
    int i;
    for ( i = start; i < (start+256); i++ )
    {
        struct instruction_type * in = &it[ i & 0xff ];
        struct compiled_instruction * ci;

        if ( in->mn == 0 )
            continue;

        if ( in->mn < I_adc )
        {
            _compile( i << 8, instruction_tables[ in->mn ] );
            continue;
        }

        ci = (struct compiled_instruction *)
            malloc( sizeof( struct compiled_instruction ));

        memset( ci, 0, sizeof( *ci ));

        if ( i < 256 )
        {
            ci->op[0] = i;
            ci->oplen = 1;
        }
        else if ( i < 65536 )
        {
            ci->op[0] = (i & 0xff00) >> 8;
            ci->op[1] = (i & 0x00ff);
            ci->oplen = 2;
        }
        else
        {
            ci->op[0] = (i & 0xff0000) >> 16;
            ci->op[1] = (i & 0x00ff00) >> 8;
            ci->op[2] = (i & 0x0000ff);
            ci->oplen = 3;
        }

        ci->mn            = instruction_types_names[ in->mn ];
        ci->mnid          = in->mn;
        ci->oper1.type    = in->type1;
        ci->oper1.val.num = in->arg1;
        ci->oper2.type    = in->type2;
        ci->oper2.val.num = in->arg2;

        *nist = ci;
        nist = &ci->next;
    }
}

void
compile_instr_list( void )
{
    instructions = 0;
    nist = &instructions;
    _compile( 0, instruction_tables[0] );
}

static void
print_oper( struct operand * oper )
{
    switch ( oper->type )
    {
    case A_NUM:
        printf( "NN" );
        break;
    case A_DEREF_REG:
        printf( "(%s)", register_types_names[ oper->val.reg ] );
        break;
    case A_DEREF_NUM:
        printf( "(NN)" );
        break;
    case A_DEREF_IX_PLUS:
        printf( "(ix+N)" );
        break;
    case A_DEREF_IY_PLUS:
        printf( "(iy+N)" );
        break;
    case A_FLAG:
        printf( "%s", flag_types_names[ oper->val.flag ] );
        break;
    case A_REG:
        printf( "%s", register_types_names[ oper->val.reg ] );
        break;
    case A_CONST:
        printf( "%d", oper->val.num );
        break;
    case A_REL:
        printf( "REL" );
        break;
    }
}

static void
print_instruction( struct compiled_instruction * ci )
{
    int i;
    for ( i = 0; i < 3; i++ )
    {
        if ( i >= ci->oplen )
            printf( "  " );
        else
            printf( "%02x", ci->op[i] );
    }
    printf( " %s ", ci->mn );
    if ( ci->oper1.type != 0 )
        print_oper( &ci->oper1 );
    if ( ci->oper2.type != 0 )
    {
        putchar( ',' );
        print_oper( &ci->oper2 );
    }
    putchar( '\n' );
}

void
print_instructions( void )
{
    struct compiled_instruction * ci;
    for ( ci = instructions; ci; ci = ci->next )
    {
        print_instruction( ci );
    }
}

static int
find_string( char ** array, char * patt, int len )
{
    int i;
    for ( i = 0; array[i] != 0; i++ )
    {
        if ( strlen( array[i] ) != len )
            continue;
        if ( memcmp( array[i], patt, strlen( array[i])) == 0 )
            return i;
    }
    return -1;
}

int
is_register( char * str, int len )
{
    return find_string( register_types_names, str, len );
}

int
is_flag( char * str, int len )
{
    return find_string( flag_types_names, str, len );
}

int
is_instruction( char * str, int len )
{
    return find_string( instruction_types_names, str, len );
}

struct compiled_instruction *
resolve_instruction( void )
{
    struct compiled_instruction * r;

 again:
    for ( r = instructions; r; r = r->next )
    {
        /* locate the mnemonic */
        if ( r->mnid == cur.mnid )
        {
            /* an instruction which takes a relative address
               gets parsed as A_NUM; convert it to A_REL */
            if ( r->oper1.type == A_REL  &&
                 cur.oper1.type == A_NUM )
            {
                cur.oper1.type = A_REL;
            }
            if ( r->oper2.type == A_REL  &&
                 cur.oper2.type == A_NUM )
            {
                cur.oper2.type = A_REL;
            }
            /* validate the operands and addressing modes */
            if ( r->oper1.type == cur.oper1.type  &&
                 r->oper2.type == cur.oper2.type )
            {
                if ( cur.oper1.type == A_DEREF_REG  ||
                     cur.oper1.type == A_REG        ||
                     cur.oper1.type == A_FLAG       ||
                     cur.oper1.type == A_CONST      )
                {
                    if ( cur.oper1.val.num != r->oper1.val.num )
                        continue;
                }
                if ( cur.oper2.type == A_DEREF_REG  ||
                     cur.oper2.type == A_REG        ||
                     cur.oper2.type == A_FLAG       ||
                     cur.oper2.type == A_CONST      )
                {
                    if ( cur.oper2.val.num != r->oper2.val.num )
                        continue;
                }
                memcpy( cur.op, r->op, sizeof( cur.op ));
                cur.oplen = r->oplen;
                return r;
            }
        }
    }

    /* check for special circumstance:
       we always parse "(ix)" as A_DEREF_IX_PLUS, however
       some instructions (such as "jp (ix)") actually enter
       them as A_DEREF_REG / R_IX.
       check for this possibility. */

    if ( cur.oper1.type == A_DEREF_IX_PLUS  &&
         cur.oper1.val.num == 0  )
    {
        cur.oper1.type = A_DEREF_REG;
        cur.oper1.val.reg = R_IX;
        goto again;
    }

    if ( cur.oper1.type == A_DEREF_IY_PLUS  &&
         cur.oper1.val.num == 0  )
    {
        cur.oper1.type = A_DEREF_REG;
        cur.oper1.val.reg = R_IY;
        goto again;
    }

    if ( cur.oper2.type == A_DEREF_IX_PLUS  &&
         cur.oper1.val.num == 0  )
    {
        cur.oper2.type = A_DEREF_REG;
        cur.oper2.val.reg = R_IX;
        goto again;
    }

    if ( cur.oper2.type == A_DEREF_IY_PLUS  &&
         cur.oper1.val.num == 0  )
    {
        cur.oper2.type = A_DEREF_REG;
        cur.oper2.val.reg = R_IY;
        goto again;
    }

    printf( "invalid instruction form: " );
    print_instruction( &cur );

    return 0;
}
