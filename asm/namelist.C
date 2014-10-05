
#include <stdio.h>
#include "z80file.H"

//
// usage:
//
// z80nm file.o
//

int
main( int argc, char ** argv )
{
    z80file f;

    if ( argc != 2 )
        return 1;

    f.read( argv[1] );
    z80header_ondisk::dump( argv[1] );

    printf( "**** symbols:\n" );
    f.dump_syms();

    printf( "**** relocations:\n" );
    f.dump_relocs();
    f.dump_sgmts();

    return 0;
}
