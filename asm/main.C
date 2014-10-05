
#include <stdio.h>
#include <string.h>
#include "z80file.H"
extern "C" {
#include "instr_table.h"
#include "instr_actions.h"
};
#include "instr_parse.H"

extern "C" void print_instructions( void );

int
main( int argc, char ** argv )
{
    FILE * input;
    char * output_file;

    compile_instr_list();

    if ( argc != 3 )
    {
    usage:
        fprintf( stderr, "usage:  z80asm file.zs file.zo\n" );
        return 1;
    }

    input = fopen( argv[1], "r" );
    if ( !input )
    {
        fprintf( stderr, "cannot open input file\n" );
        return 1;
    }
    output_file = argv[2];

    z80file object;

    char input_line[ 150 ];

    while ( fgets( input_line, 149, input ) != NULL )
    {
        int  l = strlen( input_line );
        if ( input_line[ l - 1 ] == '\r' ||
             input_line[ l - 1 ] == '\n' )
        {
            input_line[ l - 1 ] = 0;
        }
        if ( !is_empty_line( input_line )            &&
             !parse_special( &object, input_line )   &&
             !consume_label( &object, input_line )   &&
             !parse_instruction( &object, input_line ))
        {
            fprintf( stderr, "main.C parse error\n" );
            return 1;
        }
    }

    fclose( input );
    object.write( output_file );
    return 0;
}
