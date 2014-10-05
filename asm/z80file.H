
#ifndef __Z80FILE_H_
#define __Z80FILE_H_

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define UINT    unsigned int
#define UINT32  unsigned int
#define UINT64  unsigned long long
#define UINT16  unsigned short
#define UINT8   unsigned char
#define UCHAR   unsigned char

class UINT32_t
{
private:
    UCHAR p[4];
public:
    UINT32_t( void ) { }
    UINT32_t( UINT32 x ) { set( x ); }
    UINT32 get(void)
        {
            return (p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
        };
    void set( UINT32 v )
        {
            p[0] = (v >> 24) & 0xff;
            p[1] = (v >> 16) & 0xff;
            p[2] = (v >>  8) & 0xff;
            p[3] =  v    & 0xff;
        };
    int size_of(void) { return 4; }
};

// note this is little-endian, like the z80....

class UINT16_t
{
private:
    UCHAR p[2];
public:
    UINT16_t( void ) { }
    UINT16_t( UINT16 x ) { set( x ); }
    UINT16 get(void)
        {
            return (p[1]<<8) | p[0];
        };
    void set( UINT16 v )
        {
            p[1] = (v >>  8) & 0xff;
            p[0] =  v    & 0xff;
        };
    int size_of(void) { return 2; }
};

union z80flags {
    enum {
        UNDEF   =1, PRIVATE =2, PUBLIC  =3,    // 01, 02, 03
        TEXT    =1, DATA    =2, BSS     =3,    // 04, 08, 0c
        RELATIVE=0,
        RELOC16 =1, RELOC8H =2, RELOC8L =3     // 10, 20, 30
    };
    UCHAR val;
    struct {
        UCHAR def:2;
        UCHAR seg:2;
        UCHAR rel:2;
    } f;
};

struct z80header_ondisk
{
    UINT32_t magic;            // 00
    UINT32_t text_offset;      // 04
    UINT32_t data_offset;      // 08
    UINT32_t symtab_offset;    // 0c
    UINT32_t relocs_offset;    // 10

    UINT16_t text_address;     // 14
    UINT16_t data_address;     // 16
    UINT16_t bss_address;      // 18
    UINT16_t end_address;      // 1a

    UINT16_t text_size;        // 1c
    UINT16_t data_size;        // 1e
    UINT16_t bss_size;         // 20
    UINT16_t symtab_size;      // 22
    UINT16_t relocs_size;      // 24
// 26
    static void dump( char * file );
};

struct z80symbol_ondisk
{
    UCHAR len;
    z80flags flags;  // undef flag and segment type
    UINT16_t value;  // not used for undefs
    char sym[0];
};

struct z80reloc_ondisk
{
    UINT16_t symtab_offset;
    UINT16_t offset;
    z80flags flags;    // segment type and relocation type
};

enum {
    // header magic
    Z80_MAGIC_O = 0x7a38306f,
    Z80_MAGIC_E = 0x7a383065
};

struct z80reloc {
    struct z80reloc * next;
    UINT16 offset;
    z80flags flags; // segment type and relocation type
//
    z80reloc( UINT16 _off, z80flags _fl, z80reloc * _n ) {
        offset = _off; flags = _fl; next = _n;
    }
};

struct z80symbol {
    struct z80symbol * next;
    UCHAR len;
    z80flags flags; // undef flag and segment type
    UINT16 value;
    z80reloc * relocs;
    char sym[0];
//
    void * operator new( size_t s, const char * nsym ) {
        int l = strlen( nsym ) + 1;
        z80symbol * r = (z80symbol*)
            new char[ sizeof( z80symbol ) + l ];
        memcpy( r->sym, nsym, l );
        r->len = sizeof( z80symbol_ondisk ) + l;
        return (void*) r;
    };
    void operator delete( void * _p ) {
        char * p = (char*) _p;
        delete[] p;
    };
    void add_reloc( UINT16 text_offset, z80flags flags ) {
        z80reloc * r = new z80reloc( text_offset, flags, relocs );
        relocs = r;
    }
    void upgrade( void ) { flags.f.def = z80flags::PUBLIC; }
};

class z80file {
    static const int MAXSEG = 65536;
    static const int HASH_SIZE = 64;

    bool relocatable;
    UCHAR * text;
    UCHAR * data;
    UINT16 textlen;
    UINT16 datalen;
    UINT16 bsslen;
    UINT16 text_start;
    UINT16 data_start;
    UINT16 bss_start;
    UINT16 end_addr;
    z80symbol ** symbol_hash;
    int h;
    int hash( const char * sym ) {
        int s;
        for ( s = 0; *sym != 0; s += *sym ) sym++;
        return (s % HASH_SIZE);
    }

public:
    /**/        z80file      ( void );
    /**/       ~z80file      ( void );
    UINT16      current_text ( void ) { return textlen; }
    UINT16      current_data ( void ) { return datalen; }
    void        add_text     ( UCHAR * text, int len );
    void        add_data     ( UCHAR * data, int len );
    void        add_bss      ( int len ) { bsslen += len; }
    z80symbol * find_symbol  ( const char * sym );
    z80symbol * add_symbol   ( char * sym, z80flags flags );
    void        write        ( char * file );
    void        read         ( char * file );

    // NOTE!  "f2" is not usable after the link_add call!!!
    //        this function works by first killing the new file
    //        and then stealing symbols and relocs out of the carcass.
    //  if "f2" is dynamically allocated, it would be appropriate to
    //  delete it once link_add is complete.

    void        link_add     ( z80file * f2 );
    void        link_final   ( UINT16 entry );

    void        dump_syms    ( void );
    void        dump_relocs  ( void );
    void        dump_sgmts   ( void );
};

#endif
