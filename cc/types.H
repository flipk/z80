
// hmm. how to represent types?

enum {
    TYPE_NOT   = 0,
    TYPE_8BIT  = 1,
    TYPE_16BIT = 2,
    TYPE_USER  = 3
};

int     dec_to_int ( char * s, int len );
int     oct_to_int ( char * s, int len );
int     bin_to_int ( char * s, int len );
int     hex_to_int ( char * s, int len );
char *  strvec     ( char * s, int len );
int     type_verify( char * s, int len );
void    add_type   ( char * s          );
