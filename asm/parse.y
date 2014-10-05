
%{

#include <stdio.h>
#include <stdlib.h>
#include "instr_table.h"
#include "instr_actions.h"
#include "special_funcs.h"

int yylex( void );

%}

%union
{
	int val;
	char * ident;
	char * string;
	struct identifier tid;
	struct operand tval;
}

%token SPECIAL_PUBLIC SPECIAL_ASCII SPECIAL_ASCIIZ SPECIAL_DATA
%token SPECIAL_BYTE SPECIAL_WORD SPECIAL_SPACE SPECIAL_TEXT SPECIAL_BSS
%token AT_H AT_L
%token <string> STRING
%token <val> MNEM REG FLAG CONST
%token <ident> IDENT

%left '+' '-'
%left '*' '/' '%'

%type <tval> operand
%type <val> cexp
%type <tid> ident

%start line

%%

line
	: special
	| instr
	;

instr
	: MNEM
	{
	  memset( &cur, 0, sizeof( cur ));
	  cur.mnid = $1;
	  cur.mn = instruction_types_names[ $1 ];
	}
	 args
	{
	  switch ( cur.mnid )
	  {
	  case I_rst:  case I_bit:  case I_res:  
	  case I_set:  case I_im:
	    if ( cur.oper1.type == A_NUM )
	      cur.oper1.type = A_CONST;
            break;
	  }
	}
	;

args
	: /* empty */
	| operand
	{ cur.oper1 = $1; }
	| operand ',' operand
	{
	  cur.oper1 = $1;
	  cur.oper2 = $3;
	}
	;

operand
	: REG
	{ $$.type = A_REG;  $$.val.reg = $1;  $$.id.type = 0; }
	| cexp
	{ $$.type = A_NUM;  $$.val.num = $1;  $$.id.type = 0; }
	| ident
	{ $$.type = A_NUM;  $$.val.num = 0;   $$.id = $1; }
	| ident '+' cexp
	{ $$.type = A_NUM;  $$.val.num = $3;  $$.id = $1; }
	| FLAG
	{ $$.type = A_FLAG;  $$.val.flag = $1;  $$.id.type = 0; }
	| '(' cexp ')'
	{ $$.type = A_DEREF_NUM;  $$.val.num = $2;  $$.id.type = 0; }
	| '(' ident ')'
	{ $$.type = A_DEREF_NUM;  $$.val.num = 0;   $$.id = $2; }
	| '(' ident '+' cexp ')'
	{ $$.type = A_DEREF_NUM;  $$.val.num = $4;  $$.id = $2; }
	| '(' REG ')'
	{
	  if ( $2 == R_IX )
      {
          $$.type = A_DEREF_IX_PLUS;
          $$.val.num = 0;
      }
	  else if ( $2 == R_IY )
      {
          $$.type = A_DEREF_IY_PLUS;
          $$.val.num = 0;
      }
	  else
      {
          $$.type = A_DEREF_REG;
          $$.val.reg = $2;
      }
      $$.id.type = 0;
	}
	| '(' REG '+' cexp ')'
	{
	  if ( $2 != R_IX  &&  $2 != R_IY )
	  {
	    printf( "invalid register for offset\n" );
	    exit( 1 );
	  }
	  if ( $2 == R_IX )  $$.type = A_DEREF_IX_PLUS;
	  if ( $2 == R_IY )  $$.type = A_DEREF_IY_PLUS;
	  $$.val.reg = $2;  $$.val.num = $4; $$.id.type = 0;
	}
	;

cexp /* constant expression */
	: CONST
	{ $$ = $1; }
	| cexp '+' cexp
	{ $$ = $1 + $3; }
	| cexp '-' cexp
	{ $$ = $1 - $3; }
	| cexp '*' cexp
	{ $$ = $1 * $3; }
	| cexp '/' cexp
	{ $$ = $1 / $3; }
	| cexp '%' cexp
	{ $$ = $1 % $3; }
	;

ident
	: IDENT
	{ $$.type = A_IDENT_A; $$.id = $1; }
	| IDENT AT_H
	{ $$.type = A_IDENT_H; $$.id = $1; }
	| IDENT AT_L
	{ $$.type = A_IDENT_L; $$.id = $1; }
	;

special
	: SPECIAL_TEXT
	{ special_text(); }
	| SPECIAL_DATA
	{ special_data(); }
	| SPECIAL_BSS
	{ special_bss(); }
	| SPECIAL_PUBLIC IDENT
	{ special_public( $2 ); }
	| SPECIAL_ASCII STRING
	{ special_ascii( $2 ); }
	| SPECIAL_ASCIIZ STRING
	{ special_asciiz( $2 ); }
	| SPECIAL_BYTE constlist
	| SPECIAL_WORD constlist2
	| SPECIAL_SPACE CONST
	{ special_space( $2 ); }
	;

constlist
	: CONST
	{ special_const( $1 ); }
	| constlist ',' CONST
	{ special_const( $3 ); }
	;

constlist2
	: CONST
	{ special_const2( $1 ); }
	| constlist2 ',' CONST
	{ special_const2( $3 ); }
	;

%%

void
yyerror( char * s )
{
	printf( "yyerror: %s\n", s );
	exit( 1 );
}

int
yywrap( void )
{
	return 1;
}
