
#include <iostream>
#include <fstream>
#include <c++/3.3/FlexLexer.h>

#include "node.H"

using namespace std;

struct myparser {
    yyFlexLexer lexer;
    c_file out;
    myparser( ifstream * in ) : lexer( in ) { }
};

int yyparse( void * );

int
main( int argc, char ** argv )
{
    if ( argc != 2 )
    {
        printf( "usage:   ./t file.c\n" );
        return 1;
    }

    ifstream   in( argv[1], ios_base::in );
    myparser  myp( &in );
    yyparse( &myp );

    printf( "parse complete\n" );

    return 0;
}

int
yylex( void * arg )
{
    return ((myparser *)arg)->lexer.yylex();
}

c_file *
get_cfile( void * arg )
{
    return &((myparser *)arg)->out;
}
