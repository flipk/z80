
#include <stdio.h>
#include "z80file.H"

//
// usage:
//
//   z80link file1.o file2.o ... -o file.exe -e entry
//   z80link -r file1.o file2.o -o file_combined.o
//

struct cmdline {
    bool final;
    char * output;
    int maxinputs;
    int numinputs;
    int entry;
    char * inputs[0];
//
    cmdline( void ) {
        for ( int i = 0; i < maxinputs; i++ )
            inputs[i] = NULL;
        final = true;
        numinputs = 0;
        output = NULL;
        entry = -1;
    }
    void validate( void ) {
        if ( output == NULL  ||  numinputs == 0 )
        {
            printf( "bogus cmdline args\n" );
            exit( 1 );
        }
        if ( final && ( entry == -1 ))
        {
            printf( "must specify entry for a final link\n" );
            exit( 1 );
        }
    }
    void * operator new( size_t s, int args ) {
        UCHAR * p;
        int l = sizeof( cmdline ) + (args * 4);
        p = new UCHAR[ l ];
        cmdline * _p = (cmdline *)p;
        _p->maxinputs = args;
        return (void*) p;
    };
    void operator delete( void * _p ) {
        UCHAR * p = (UCHAR *)_p;
        delete[] p;
    }
};

int
axtoi( char * p )
{
    static const char digs[] =
    {
/*0x30*/   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
/*0x40*/  -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/*0x50*/  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/*0x60*/  -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/*0x70*/  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    };
    int r = 0;
    if ( p[0] == '0' && p[1] == 'x' )
    {
        // convert hex
        p += 2;
        while ( *p )
        {
            r <<= 4;
            int v = *p++;
            if ( v < '0' || v > 0x7f )
            {
                printf( "bogus hex digit!\n" );
                exit( 1 );
            }
            r += digs[ v - '0' ];
        }
    }
    else // decimal
    {
        while ( *p )
        {
            r *= 10;
            int v = *p++;
            if ( v < '0' || v > '9' )
            {
                printf( "bogus decimal digit!\n" );
                exit( 1 );
            }
            r += digs[ v - '0' ];
        }
    }
    return r;
}

int
main( int argc, char ** argv )
{
    cmdline * cmd;

    cmd = new( argc ) cmdline;

    argv++;
    for ( ; argc > 1; argc--, argv++ )
    {
        if ( strcmp( argv[0], "-r" ) == 0 )
        {
            cmd->final = false;
        }
        else if ( strcmp( argv[0], "-o" ) == 0 )
        {
            cmd->output = argv[1];
            argc--; argv++;
        }
        else if ( strcmp( argv[0], "-e" ) == 0 )
        {
            cmd->entry = axtoi( argv[1] );
            argc--; argv++;
        }
        else
        {
            cmd->inputs[ cmd->numinputs++ ] = argv[0];
        }
    }

    cmd->validate();

    z80file * out = new z80file;

    out->read( cmd->inputs[0] );
    for ( int i = 1; i < cmd->numinputs; i++ )
    {
        z80file in;
        in.read( cmd->inputs[i] );
        out->link_add( &in );
    }

    if ( cmd->final )
        out->link_final( cmd->entry );

    out->write( cmd->output );
    delete out;

    return 0;
}
