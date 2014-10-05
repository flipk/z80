
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.H"

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

int
oct_to_int ( char * s, int len )
{
    int r = 0;
    while ( len-- > 0 )
    {
        r *= 8;
        r += (*s) - '0';
        s++;
    }
    return r;
}

int
bin_to_int ( char * s, int len )
{
    int r = 0;
    while ( len-- > 0 )
    {
        r *= 2;
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


static int types_initialized = 0;

#define HASH_SIZE 32
static int
calchash( char * s )
{
    int r;
    for ( r = 0; *s; s++ )
        r += *s;
    return r % HASH_SIZE;
}

struct name {
    struct name * next;
    int val;
    char name[0];
};

static struct name * hash[ HASH_SIZE ];
static int last_type;

static struct name *
hash_find( char * s )
{
    int h = calchash( s );
    struct name * r;
    for ( r = hash[h]; r; r = r->next )
        if ( strcmp( r->name, s ) == 0 )
            return r;
    return NULL;
}

static void
hash_add( char * s, int val )
{
    int len, h;
    struct name * n;
    len = strlen( s ) + 1;
    n = (struct name *) malloc( sizeof( struct name ) + len );
    h = calchash( s );
    n->next = hash[h];
    hash[h] = n;
    strcpy( n->name, s );
    n->val = val;
}

static void
init_types( void )
{
    int i;
    last_type = TYPE_USER;
    for ( i = 0; i < HASH_SIZE; i++ )
        hash[i] = NULL;
    hash_add( "int8", TYPE_8BIT );
    hash_add( "int16", TYPE_16BIT );
    types_initialized = 1;
}

int
type_verify( char * s, int len )
{
    struct name * n;
    if ( types_initialized == 0 )
        init_types();
    n = hash_find( s );
    if ( n )
        return n->val;
    return TYPE_NOT;
}

/*
 * need way to add type and way to allocate type numbers
 * starting at TYPE_USER
 */

void
add_type( char * s )
{
    hash_add( s, last_type++ );
}
