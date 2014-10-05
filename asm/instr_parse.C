
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "z80file.H"
extern "C" {
#include "instr_table.h"
#include "instr_actions.h"
#include "special_funcs.h"
void yyparse( void );
int my_yyinput( char * buf, int max_size );
};
#include "instr_parse.H"

#define DEBUG 0

static char * mybuf;
static int mybufpos;
static int mybuflen;
static UCHAR current_seg = 0;

// this is used by the parser/lexer.
// the parser is setup to work for only one instruction.

int
my_yyinput( char * buf, int max_size )
{
    if ( max_size > mybuflen )
        max_size = mybuflen;
    if ( max_size == 0 )
        return 0;
    memcpy( buf, mybuf + mybufpos, max_size );
    mybufpos += max_size;
    mybuflen -= max_size;
    return max_size;
}

// co  = compiled operand
// ito = instruction table operand

static bool
handle_operand( z80file * object, int which, operand * co, operand * ito )
{

    switch ( ito->type )
    {
    case A_NUM:
    case A_DEREF_NUM:
    case A_DEREF_IX_PLUS:
    case A_DEREF_IY_PLUS:
    case A_REL:
#if DEBUG==1
        printf( "arg%d length : %d\n", which, ito->val.num );
#endif
        if ( ito->val.num == 1  &&   co->val.num > 255 )
        {
            fprintf( stderr, "argument value does not fit in a byte\n" );
            return false;
        }
        if ( co->id.id )
        {
#if DEBUG==1
            printf( "arg%d symbol : '%s'\n", which, co->id.id );
#endif

            z80flags flags;
            flags.f.def = z80flags::UNDEF;
            flags.f.seg = 0;
            flags.f.rel = 0;
            z80symbol * s = object->add_symbol( co->id.id, flags );
            flags.f.def = 0;
            flags.f.seg = z80flags::TEXT;

            if ( ito->type == A_REL )
                flags.f.rel = z80flags::RELATIVE;
            else if ( ito->val.num == 2 )
                flags.f.rel = z80flags::RELOC16;
            else if ( co->id.type == A_IDENT_L )
                flags.f.rel = z80flags::RELOC8L;
            else if ( co->id.type == A_IDENT_H )
                flags.f.rel = z80flags::RELOC8H;
            else
            {
                fprintf( stderr, "internal error in parse\n" );
                return false;
            }

            s->add_reloc( object->current_text(), flags );
        }

        UCHAR n = (co->val.num & 0xff);
        object->add_text( &n, 1 );

        if ( ito->val.num == 2 )
        {
            n = (co->val.num & 0xff00) >> 8;
            object->add_text( &n, 1 );
        }
#if DEBUG==1
        printf( "arg%d value : %d\n", which, co->val.num );
#endif
        break;
    }

    return true;
}

bool
parse_instruction( z80file * object, char * instr )
{
    struct compiled_instruction * ci;
    int i;

    if ( instr[0] != '\t' && instr[0] != ' ' )
        return false;

    mybuf = instr;
    mybufpos = 0;
    mybuflen = strlen( instr );

    yyparse();
    ci = resolve_instruction();

    /*
     * at this point, "cur" contains the parsed values of the
     * instruction, and "ci" points into the instruction table
     * at the mnemonic and argument information.
     */

    if ( ci == 0 )
        return false;

    if ( current_seg != z80flags::TEXT )
    {
        fprintf( stderr, "instructions can only be added to text segment\n" );
        exit( 1 );
    }

#if DEBUG==1
    printf( "opcode: " );
    for ( i = 0; i < cur.oplen; i++ )
        printf( "%02x ", cur.op[i] );
    printf( "\n" );
#endif

    object->add_text( cur.op, cur.oplen );

    if ( !handle_operand( object, 1, &cur.oper1, &ci->oper1 ))
        return false;

    if ( !handle_operand( object, 2, &cur.oper2, &ci->oper2 ))
        return false;

    return true;
}

bool
is_empty_line( char * line )
{
    // skip any whitespace
    while ( *line == ' ' || *line == '\t' )
        line++;
    // look for a comment or eol
    if ( *line == ';' || *line == '#' || *line == 0 )
        return true;
    return false;
}

extern "C" {
    int     dec_to_int ( char * s, int len );
    int     hex_to_int ( char * s, int len );
};

static z80file * special_object;

bool
parse_special( z80file * object, char * line )
{
    while ( *line == ' ' || *line == '\t' )
        line++;
    if ( line[0] != '.' )
        return false;

    mybuf = line;
    mybufpos = 0;
    mybuflen = strlen( line );

    special_object = object;
    yyparse();
    return true;
}

void
special_public( char * sym )
{
    z80symbol * s = special_object->find_symbol( sym );
    if ( !s )
    {
        fprintf( stderr, "can't find symbol '%s' to make public\n", sym );
        exit( 1 );
    }
    s->upgrade();
}

void
special_ascii( char * str )
{
    if ( current_seg != z80flags::DATA  &&
         current_seg != z80flags::TEXT )
    {
        fprintf( stderr, "ascii can only be added to "
                 "text or data segments\n" );
        exit( 1 );
    }
    special_object->add_data( (UCHAR*)str, strlen( str ));
}

void
special_asciiz( char * str )
{
    if ( current_seg != z80flags::DATA  &&
         current_seg != z80flags::TEXT )
    {
        fprintf( stderr, "ascii can only be added to "
                 "text or data segments\n" );
        exit( 1 );
    }
    special_object->add_data( (UCHAR*)str, strlen( str ) + 1);
}

void
special_const( int v )
{
    if ( current_seg != z80flags::DATA  &&
         current_seg != z80flags::TEXT )
    {
        fprintf( stderr, "constant data can only be added to "
                 "text or data segments\n" );
        exit( 1 );
    }
    if ( v > 255 )
    {
        fprintf( stderr, "value %d does not fit in a byte\n", v );
        exit( 1 );
    }   
    UCHAR b = v & 0xff;
    special_object->add_data( &b, 1 );
}

void
special_const2( int v )
{
    if ( current_seg != z80flags::DATA  &&
         current_seg != z80flags::TEXT )
    {
        fprintf( stderr, "constant data can only be added to "
                 "text or data segments\n" );
        exit( 1 );
    }
    if ( v > 65535 )
    {
        fprintf( stderr, "value %d does not fit in a word\n", v );
        exit( 1 );
    }   
    UCHAR b = v & 0xff;
    special_object->add_data( &b, 1 );
    b = v & 0xff00;
    special_object->add_data( &b, 1 );
}

void
special_space( int v )
{
    if ( current_seg != z80flags::BSS )
    {
        fprintf( stderr, ".space can only be added to bss segment\n" );
        exit( 1 );
    }
    special_object->add_bss( v );
}

void
special_text( void )
{
    current_seg = z80flags::TEXT;
}

void
special_data( void )
{
    current_seg = z80flags::DATA;
}

void
special_bss( void )
{
    current_seg = z80flags::BSS;
}

bool
consume_label( z80file * object, char * line )
{
    int l = 0;

    if ( !isalpha( line[0] ))
        return false;

    if ( current_seg == 0 )
    {
        fprintf( stderr, "no segment was specified\n" );
        exit( 1 );
    }

    while ( isalpha( line[l] ) || isdigit( line[l] ) || line[l] == '_' )
        l++;

    line[l] = 0;
    z80flags flags;
    flags.f.def = z80flags::PRIVATE;
    flags.f.seg = current_seg;
    flags.f.rel = 0;
    object->add_symbol( line, flags );

    return true;
}
