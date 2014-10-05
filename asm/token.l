
%{

#include "instr_table.h"
#include "y.tab.h"

int my_yyinput( char * buf, int max_size );

#define YY_INPUT(buf,result,max_size) \
        result = my_yyinput( buf, max_size )

int yywrap( void );

#define DEBUG 0
#if DEBUG==1
#define RET1(v) do { printf( " %s\n", #v ); return v; } while(0)
#define RET2(v) do { printf( " '%c'\n", v ); return v; } while(0)
#define RET3(v,s) do { printf( " %s ", #v ); printf s; return v; } while(0)
#else
#define RET1(v) return v
#define RET2(v) return v
#define RET3(v,s) return v
#endif

int     dec_to_int ( char * s, int len );
int     hex_to_int ( char * s, int len );
char *  strvec     ( char * s, int len );

%}

%%

\.public	{ RET1(SPECIAL_PUBLIC); }
\.asciiz	{ RET1(SPECIAL_ASCIIZ); }
\.ascii		{ RET1(SPECIAL_ASCII); }
\.byte		{ RET1(SPECIAL_BYTE); }
\.word		{ RET1(SPECIAL_WORD); }
\.space		{ RET1(SPECIAL_SPACE); }
\.text		{ RET1(SPECIAL_TEXT); }
\.data		{ RET1(SPECIAL_DATA); }
\.bss		{ RET1(SPECIAL_BSS); }
\"[^\"]*\"	{ yylval.string = strvec( yytext+1, yyleng-2 ); RET1(STRING); }
[0-9]+		{ yylval.val = dec_to_int( yytext, yyleng ); RET1(CONST); }
0x[0-9a-fA-F]+	{ yylval.val = hex_to_int( yytext, yyleng ); RET1(CONST); }
[ \t\r\n]*	{ /* ignore whitespace */ }
;.*		{ /* discard a comment */ }
@h		{ RET1(AT_H); }
@l		{ RET1(AT_L); }
[a-zA-Z][a-zA-Z0-9_']* {
		  int v;
		  if (( v = is_instruction( yytext, yyleng )) != -1 )
		  {
		    yylval.val = v;
		    RET3(MNEM,( "'%s'\n", yytext ));
		  }
		  if (( v = is_register( yytext, yyleng )) != -1 )
		  {
		    yylval.val = v;
		    RET3(REG,( "'%s'\n", yytext ));
		  }
		  if (( v = is_flag( yytext, yyleng )) != -1 )
		  {
		    yylval.val = v;
		    RET3(FLAG,( "'%s'\n", yytext ));
		  }
		  yylval.ident = strvec( yytext, yyleng );
		  RET3(IDENT,( "'%s'\n", yytext ));
		}
.		{ int v = yytext[0]; RET2(v); }

%%

int
dec_to_int( char * s, int len )
{
	int r = 0;
	while ( len-- > 0 )
	{
		r *= 10;
		r += (*s) - '0';
		s++;
	}
	return r;
}

int
hex_to_int( char * s, int len )
{
	int r = 0;
	s += 2;
	len -= 2;
	while ( len-- > 0 )
	{
		r *= 16;
		if ( (*s) >= 'a' && (*s) <= 'f' )
			r += (*s) - 'a' + 10;
		else if ( (*s) >= 'A' && (*s) <= 'F' )
			r += (*s) - 'A' + 10;
		else
			r += (*s) - '0';
		s++;
	}
	return r;
}

#define MAXRETS 10
#define MAXRETLEN 30

static char strvecrets[ MAXRETS ][ MAXRETLEN+1 ];
static int curstrvecret = 0;

char *
strvec( char * s, int len )
{
	char * r;
	r = strvecrets[ curstrvecret ];
	curstrvecret = (curstrvecret + 1) % MAXRETS;
	if ( len > MAXRETLEN )
	{
		printf( "*** error, ident too long\n" );
		exit( 1 );
	}
	memcpy( r, s, len );
	r[len] = 0;
	return r;
}
