
%{

/* xxx enum constants? */

#include "node.H"
#include "c-parse.tab.H"
#include "types.H"

#if 0

#define RET1(v)                   \
    do {                          \
        printf( " %s \n", #v );   \
        return v;                 \
    } while ( 0 )

#define RET2(v,s)                 \
    do {                          \
        printf( " %s ", #v );     \
        printf s;                 \
        printf( "\n" );           \
        return v;                 \
    } while ( 0 )

#else
#define RET1(v) return v
#define RET2(v,s) return v
#endif

%}

hex		0x[0-9a-fA-F]+
binary		0b[01]+
octal		0[0-7]+
decimal		[0-9]+
white		[ \t\r\n]+
string		\"[^\"]*\"
word		[a-zA-Z][a-zA-Z0-9_]*

%%

\/\*		{
			int c;
			bool done = false;
			while ( !done )
			{
				c = yyinput();
				if ( c == EOF )
					return EOF;
				while ( !done && c == '*' )
				{
					c = yyinput();
					if ( c == '/' || c == 0 )
						done = true;
					else if ( c != '*' )
						break;
				}
			}
		}

{white}		{ /* skip */ }
\/\/.*$		{ /* skip */ }

{hex}		{ yylval.val = hex_to_int( yytext, yyleng );
		  RET1(TOK_CONST); }
{binary}	{ yylval.val = bin_to_int( yytext, yyleng );
		  RET1(TOK_CONST); }
{octal}		{ yylval.val = oct_to_int( yytext, yyleng );
		  RET1(TOK_CONST); }
{decimal}	{ yylval.val = dec_to_int( yytext, yyleng );
		  RET1(TOK_CONST); }
{string}	{ char * s = (char*) malloc( yyleng );
		  memcpy( s, yytext+1, yyleng-2 );
		  s[yyleng-2] = 0;
		  yylval.str = s;
		  RET1(TOK_STRING); }

\{		{ RET1(TOK_LB); }
\}		{ RET1(TOK_RB); }
\(		{ RET1(TOK_LP); }
\)		{ RET1(TOK_RP); }
\;		{ RET1(TOK_SEMI); }
\:		{ RET1(TOK_COLON); }
\.		{ RET1(TOK_DOT); }
->		{ RET1(TOK_ARROW); }
\!\=		{ RET1(TOK_NOTEQ); }
\!		{ RET1(TOK_BANG); }
\=\=		{ RET1(TOK_EQEQ); }
\=		{ RET1(TOK_EQ); }
\~\=		{ RET1(TOK_TWID_EQ); }
\~		{ RET1(TOK_TWID); }
\*\=		{ RET1(TOK_STAREQ); }
\*		{ RET1(TOK_STAR); }
\/\=		{ RET1(TOK_SLASHEQ); }
\/		{ RET1(TOK_SLASH); }
\%\=		{ RET1(TOK_MODEQ); }
\%		{ RET1(TOK_MOD); }
\+\=		{ RET1(TOK_PLUSEQ); }
\+\+		{ RET1(TOK_PLUSPLUS); }
\+		{ RET1(TOK_PLUS); }
\-\=		{ RET1(TOK_MINUSEQ); }
\-\-		{ RET1(TOK_MINUSMINUS); }
\-		{ RET1(TOK_MINUS); }
\<\<\=		{ RET1(TOK_LSHIFTEQ); }
\<\=		{ RET1(TOK_LTEQ); }
\<\<		{ RET1(TOK_LSHIFT); }
\<		{ RET1(TOK_LT); }
\>\>\=		{ RET1(TOK_RSHIFTEQ); }
\>\=		{ RET1(TOK_GTEQ); }
\>\>		{ RET1(TOK_RSHIFT); }
\>		{ RET1(TOK_GT); }
\&\=		{ RET1(TOK_ANDEQ); }
\&\&		{ RET1(TOK_ANDAND); }
\&		{ RET1(TOK_AND); }
\^\=		{ RET1(TOK_XOREQ); }
\^		{ RET1(TOK_XOR); }
\|\=		{ RET1(TOK_OREQ); }
\|\|		{ RET1(TOK_OROR); }
\|		{ RET1(TOK_OR); }
\?		{ RET1(TOK_QUEST); }
\[		{ RET1(TOK_LSB); }
\]		{ RET1(TOK_RSB); }
,		{ RET1(TOK_COMMA); }

if		{ RET1(TOK_IF); }
else		{ RET1(TOK_ELSE); }
void		{ RET1(TOK_VOID); }
struct		{ RET1(TOK_STRUCT); }
break		{ RET1(TOK_BREAK); }
switch		{ RET1(TOK_SWITCH); }
case		{ RET1(TOK_CASE); }
default		{ RET1(TOK_DEFAULT); }
enum		{ RET1(TOK_ENUM); }
return		{ RET1(TOK_RETURN); }
continue	{ RET1(TOK_CONTINUE); }
for		{ RET1(TOK_FOR); }
sizeof		{ RET1(TOK_SIZEOF); }
do		{ RET1(TOK_DO); }
static		{ RET1(TOK_STATIC); }
extern		{ RET1(TOK_EXTERN); }
while		{ RET1(TOK_WHILE); }

{word}		{
		   int t;
		   char * s = strvec( yytext, yyleng );
		   t = type_verify( yytext, yyleng );
		   if ( t == TYPE_NOT )
		   {
		      yylval.str = s;
		      RET2(TOK_IDENT,(": %s", s));
		   }
		   yylval.val = t;
		   RET2(TOK_TYPE,(": %s", s));
		}

%%
