
%{

/* xxx enum constants? */

#include <stdio.h>
#include <stdlib.h>
#include "types.H"
#include "node.H"

int yylex( void * lexer );
c_file * get_cfile( void * lexer );

extern "C" {
void yyerror( char * s );
int  yywrap( void );
};

#define   YYLEX_PARAM myp
#define YYPARSE_PARAM myp

#define OUT get_cfile(myp)

%}

%union
{
	char * str;
	int val;
	c_node_base * node;
}

%token TOK_LB TOK_RB TOK_LP TOK_RP TOK_DOT TOK_ARROW TOK_BANG
%token TOK_SEMI TOK_COLON TOK_COMMA TOK_STAR
%token TOK_AND TOK_ANDAND TOK_OR TOK_OROR
%token TOK_EQ TOK_TWID_EQ TOK_TWID TOK_STAREQ
%token TOK_SLASHEQ TOK_SLASH TOK_MODEQ TOK_MOD
%token TOK_PLUSEQ TOK_PLUSPLUS TOK_PLUS TOK_MINUSEQ TOK_MINUSMINUS
%token TOK_MINUS TOK_LSHIFTEQ TOK_LSHIFT TOK_RSHIFTEQ TOK_RSHIFT
%token TOK_ANDEQ TOK_XOREQ TOK_XOR TOK_OREQ TOK_QUEST TOK_LSB
%token TOK_RSB TOK_BREAK TOK_SWITCH TOK_CASE TOK_DEFAULT TOK_ENUM
%token TOK_RETURN TOK_CONTINUE TOK_FOR TOK_SIZEOF TOK_DO TOK_STATIC
%token TOK_EXTERN TOK_WHILE TOK_EQEQ TOK_NOTEQ TOK_GT TOK_LT TOK_GTEQ
%token TOK_LTEQ TOK_TYPE TOK_STRING TOK_VOID TOK_IF TOK_ELSE TOK_STRUCT

%token <str> TOK_IDENT
%token <str> TOK_STRING   /* malloc'd memory! */
%token <val> TOK_CONST

%nonassoc  TOK_IF
%nonassoc  TOK_ELSE
%left      TOK_IDENT TOK_TYPE TOK_VOID TOK_ENUM 
%left      TOK_LB TOK_COMMA TOK_SEMI
%right     TOK_COLON
%right     TOK_EQ
%right     TOK_TWID_EQ TOK_STAREQ TOK_SLASHEQ TOK_MODEQ TOK_PLUSEQ TOK_MINUSEQ TOK_LSHIFTEQ TOK_RSHIFTEQ TOK_ANDEQ TOK_XOREQ TOK_OREQ
%right     TOK_QUEST
%left      TOK_OROR
%left      TOK_ANDAND
%left      TOK_OR
%left      TOK_XOR
%left      TOK_AND
%left      TOK_EQEQ
%left      TOK_LT TOK_GT TOK_LTEQ TOK_GTEQ TOK_NOTEQ
%left      TOK_LSHIFT TOK_RSHIFT
%left      TOK_PLUS TOK_MINUS
%left      TOK_STAR TOK_SLASH TOK_MOD
%left      UNARY TOK_PLUSPLUS TOK_MINUSMINUS TOK_TWID
%left      TOK_ARROW TOK_DOT TOK_LP TOK_LSB

%type <node> expr expr_comma structvarlist structvar type stars
%type <node> casestmt caselist stmt stmts arraydim arraydef
%type <node> toplevvardef vardef vardefs block block_or_stmt
%type <node> parmdecl parmdecls parmlist protofunc var_init
%type <node> const_init_list const_init string

%start start

%%

start
	: /* empty */
	| toplvls
	;

toplvls
	: toplvl
	| toplvls toplvl
	;

toplvl
	: structdef
	| toplevvardef
	| TOK_STATIC toplevvardef
	| TOK_EXTERN toplevvardef
	| protofunc
	;

structdef
	: TOK_STRUCT TOK_IDENT TOK_LB structvarlist TOK_RB TOK_SEMI
	{
		add_type( $2 ); 
//		OUT->types.add( 0 );	
	}
	;

structvarlist
	: structvar
	| structvarlist structvar
	;

structvar
	: type TOK_IDENT TOK_SEMI
	;

type
	: TOK_VOID stars
	{
		c_node_simpletype * c = new c_node_simpletype;
		c->name = "type:void";
		c->bitwidth = 0;
		c->is_signed = false;

		// find the last pointertype from the stars list
		// and insert the void simpletype at the end.

		c_node_pointertype * pc;
		for (	pc = (c_node_pointertype*) $2;
			pc->deref != 0;
			pc = (c_node_pointertype*) pc->deref )
			;

		pc->deref = c;
		$$ = $2;
	}
	| TOK_TYPE stars
	{
		// $1 should somehow be the type
		// xxx repeat the above code
	}
	| TOK_TYPE
	{
		// $1 should somehow be the type
		// xxx
		// $$ = $1;
	}
	;

stars
	: TOK_STAR
	{
		c_node_pointertype * c = new c_node_pointertype;
		c->name = "star";
		c->dimension = -1;
		// c->deref is not populated!
		c->deref = 0;
		$$ = c;
	}
	| stars TOK_STAR
	{
		c_node_pointertype * c = new c_node_pointertype;
		c->name = "star";
		c->dimension = -1;
		((c_node_pointertype *)$1)->deref = c;
		// c->deref is not populated!
		c->deref = 0;
		$$ = $1;
	}
	;

const_init
	: TOK_CONST
	{
		c_node_const * c = new c_node_const;
		c->value = $1;
		$$ = c;
	}
	| string
	| TOK_LB const_init_list TOK_RB
	{}
	;

const_init_list
	: const_init
	| const_init_list TOK_COMMA const_init
	;

var_init
	: expr
	| TOK_LB const_init_list TOK_RB
	{}
	;

protofunc
	:            type TOK_IDENT TOK_LP parmlist TOK_RP TOK_SEMI
	| TOK_EXTERN type TOK_IDENT TOK_LP parmlist TOK_RP TOK_SEMI
	{}
	|            type TOK_IDENT TOK_LP parmlist TOK_RP block
	| TOK_STATIC type TOK_IDENT TOK_LP parmlist TOK_RP block
	{}
	;

parmlist
	: TOK_VOID
	{}
	| parmdecls
	;

parmdecls
	: parmdecl
	| parmdecl TOK_COMMA parmdecls
	;

parmdecl
	: type  TOK_IDENT
	;

block_or_stmt
	: block
	| stmt
	;

block
	: TOK_LB vardefs stmts TOK_RB
	{}
	| TOK_LB         stmts TOK_RB
	{}
	| TOK_LB vardefs       TOK_RB   /* a stupid construct, but legal */
	{}
	| TOK_LB               TOK_RB
	{}
	;

vardefs
	: vardef
	| vardefs vardef
	;

vardef
	: type TOK_IDENT TOK_SEMI
	| type TOK_IDENT TOK_EQ var_init TOK_SEMI
	| type TOK_IDENT arraydef TOK_EQ var_init TOK_SEMI
	;

toplevvardef
	: type TOK_IDENT TOK_SEMI
	| type TOK_IDENT TOK_EQ const_init TOK_SEMI
	| type TOK_IDENT arraydef TOK_EQ const_init TOK_SEMI
	;

arraydef
	: arraydim
	| arraydef arraydim
	;

arraydim
	: TOK_LSB TOK_CONST TOK_RSB
	{}
	;

stmts
	: stmt
	| block
	| stmts stmt
	| stmts block
	;

stmt
	: expr TOK_SEMI
	| TOK_IF TOK_LP expr TOK_RP block_or_stmt   %prec TOK_IF
	{}
	| TOK_IF TOK_LP expr TOK_RP block_or_stmt TOK_ELSE block_or_stmt
	{}
	| TOK_BREAK TOK_SEMI
	{}
	| TOK_SWITCH TOK_LP expr TOK_RP TOK_LB          TOK_RB
	{}
	| TOK_SWITCH TOK_LP expr TOK_RP TOK_LB caselist TOK_RB
	{}
	| TOK_RETURN TOK_SEMI
	{}
	| TOK_RETURN expr TOK_SEMI
	{}
	| TOK_CONTINUE TOK_SEMI
	{}
	| TOK_FOR TOK_LP expr_comma TOK_SEMI
	  expr TOK_SEMI expr_comma TOK_RP block_or_stmt
	{}
	| TOK_DO block_or_stmt TOK_WHILE TOK_LP expr TOK_RP TOK_SEMI
	{}
	| TOK_WHILE TOK_LP expr TOK_RP block_or_stmt
	{}
	;

caselist
	: casestmt
	| caselist casestmt
	;

casestmt
	: TOK_CASE TOK_CONST TOK_COLON
	{}
	| TOK_DEFAULT TOK_COLON
	{}
	| block_or_stmt
	;

expr_comma
	: expr
	| expr_comma TOK_COMMA expr
	;

string
	: TOK_STRING
	{
		/* note that TOK_STRING returns malloc'd memory */
		c_node_stringconst * c = new c_node_stringconst;
		c->string = $1;
		$$ = c;
	}
	;

expr
	: TOK_CONST
	{
		c_node_const * c = new c_node_const;
		c->value = $1;
		$$ = c;
	}
	| TOK_IDENT
	{
		/* xxx search stack of current contexts
		   looking for IDENT, then create a INSTR_VARREF
		   node referencing the var in the correct context. */
	}
	| string
	{ $$ = $1; }
	| expr TOK_DOT   TOK_IDENT
	{
		/* xxx search 'mytype' field of expr for a type
		   named by IDENT, but only if expr->mytype is NOT
		   a ptr-to type.  if its a ptr-to type, then TOK_DOT
		   is illegal.  construct a INSTR_VARREF and return. */
	}
	| expr TOK_ARROW TOK_IDENT
	{
		/* xxx search expr->mytype for a type
		   named by IDENT, but only if expr->mytype IS
		   a ptr-to type.  if its not a ptr-to type, then TOK_ARROW
		   is illegal. */
	}
	| expr       TOK_LSB expr       TOK_RSB
	{
		
	}
	|            TOK_LP  expr       TOK_RP
	{}
	| expr       TOK_LP  expr_comma TOK_RP
	| expr       TOK_LP             TOK_RP
	| TOK_SIZEOF TOK_LP  expr       TOK_RP
	{}

	| TOK_AND            expr   %prec UNARY
	{}
	| TOK_STAR           expr   %prec UNARY
	{}
	| TOK_BANG           expr   %prec UNARY
	{}
	| TOK_TWID           expr   %prec UNARY
	{}
	| TOK_MINUS          expr   %prec UNARY
	{}
	| TOK_PLUSPLUS       expr   %prec UNARY
	{}
	| TOK_MINUSMINUS     expr   %prec UNARY
	{}
	| TOK_LP type TOK_RP expr   %prec UNARY
	{}

	| expr TOK_PLUSPLUS
	| expr TOK_MINUSMINUS

	| expr TOK_QUEST  expr TOK_COLON expr
	| expr TOK_AND    expr
	| expr TOK_ANDAND expr
	| expr TOK_OR     expr
	| expr TOK_OROR   expr
	| expr TOK_SLASH  expr
	| expr TOK_MOD    expr
	| expr TOK_PLUS   expr
	| expr TOK_MINUS  expr
	| expr TOK_LSHIFT expr
	| expr TOK_RSHIFT expr
	| expr TOK_XOR    expr

	| expr TOK_EQEQ   expr
	| expr TOK_NOTEQ  expr
	| expr TOK_GT     expr
	| expr TOK_LT     expr
	| expr TOK_GTEQ   expr
	| expr TOK_LTEQ   expr

	| expr TOK_TWID_EQ  expr
	| expr TOK_STAREQ   expr
	| expr TOK_SLASHEQ  expr
	| expr TOK_MODEQ    expr
	| expr TOK_PLUSEQ   expr
	| expr TOK_MINUSEQ  expr
	| expr TOK_LSHIFTEQ expr
	| expr TOK_RSHIFTEQ expr
	| expr TOK_ANDEQ    expr
	| expr TOK_XOREQ    expr
	| expr TOK_OREQ     expr
	| expr TOK_EQ       expr

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
