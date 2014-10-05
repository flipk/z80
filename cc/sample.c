
struct one_t {
    int8 j;
};

struct two_t {
    int16 k;
};

struct three_t {
    one_t   a;
    two_t * b;
    int16   c;
};

int8  var1 ;
int8  var2 = 4;
int16 var3 ;
int16 var4 = 1234;

int16 func ( int8  one,  int16  two, two_t * three );

int16
func( void ** x  )
{
    int8 a;
    int8 b;
    int16 * c;
    three_t * d;
    int ( *e ) ( int );

    switch ( a )
    {
    case 1:
        printf( "junk", a, b );
        break;

    case 2:
        a = b;
        c = (int16*)4;
        break;

    default:
        break;
    }

    c = func( a, *c );
    d = (three_t*) c;
    *c = d->b->k;
    d = (three_t*)&(c[24]);

    d = (*e)( d );

    return c ^ 1;
}
