
#include <stdio.h>
#include "z80file.H"

// static
void
z80header_ondisk :: dump( char * file )
{
    FILE * f;
    f = fopen( file, "r" );
    if ( !f )
    {
        fprintf( stderr, "cannot open file\n" );
        exit( 1 );
    }

    z80header_ondisk zh;
    if ( fread( &zh, sizeof( zh ), 1, f ) != 1 )
    {
        fprintf( stderr, "cannot read header\n" );
        exit( 1 );
    }

    if ( zh.magic.get() == Z80_MAGIC_O )
        printf( "object type:  z80 unlinked object file\n" );
    else if ( zh.magic.get() == Z80_MAGIC_E )
        printf( "object type:  z80 linked executable file\n" );
    else
    {
        fprintf( stderr, "error, no z80 object file magic found\n" );
        exit( 1 );
    }

    printf( "text offset: %#x   text size: %#x\n",
            zh.text_offset.get(), zh.text_size.get() );
    printf( "data offset: %#x   data size: %#x\n",
            zh.data_offset.get(), zh.data_size.get() );
    printf( "bss size: %#x\n", zh.bss_size.get() );
    printf( "symtab offset: %#x   symtab size: %#x\n",
            zh.symtab_offset.get(), zh.symtab_size.get() );
    printf( "relocs offset: %#x   relocs size: %#x\n",
            zh.relocs_offset.get(), zh.relocs_size.get() );

    if ( zh.magic.get() == Z80_MAGIC_E )
    {
        printf( "text address: %#x   data address: %#x\n",
                zh.text_address.get(), zh.data_address.get() );
        printf( "bss address: %#x    end address: %#x\n",
                zh.bss_address.get(),  zh.end_address.get() );
    }

    fclose( f );
}

z80file :: z80file( void )
{
    text = new UCHAR[ MAXSEG ];
    data = new UCHAR[ MAXSEG ];
    textlen = 0;
    datalen = 0;
    bsslen = 0;
    symbol_hash = new z80symbol*[ HASH_SIZE ];
    memset( symbol_hash, 0, sizeof( z80symbol*[ HASH_SIZE ] ));
    relocatable = true;
    text_start = 0;
    data_start = 0;
    bss_start = 0;
    end_addr = 0;
}

z80file :: ~z80file( void )
{
    z80symbol * s, * ns;
    int i;

    for ( i = 0; i < HASH_SIZE; i++ )
        for ( s = symbol_hash[ i ], ns = NULL; s; s = ns )
        {
            ns = s->next;
            delete s;
        }

    delete[] text;
    delete[] symbol_hash;
}

#define BAIL(x) { printf x; exit( 1 ); }

void
z80file :: add_text    ( UCHAR * in_text, int len )
{
    UINT32 nsize = textlen;
    if (( nsize + len ) >= MAXSEG )
        BAIL(( "text segment overflow\n" ));
    memcpy( text + textlen, in_text, len );
    textlen += len;
}

void
z80file :: add_data    ( UCHAR * in_data, int len )
{
    UINT32 nsize = datalen;
    if (( nsize + len ) >= MAXSEG )
        BAIL(( "data segment overflow\n" ));
    memcpy( data + datalen, in_data, len );
    datalen += len;
}

z80symbol *
z80file :: find_symbol ( const char * sym )
{
    z80symbol * n;
    h = hash( sym );

    for ( n = symbol_hash[h]; n; n = n->next )
        if ( strcmp( n->sym, sym ) == 0 )
            break;

    return n;
}

z80symbol *
z80file :: add_symbol ( char * sym, z80flags flags )
{
    z80symbol * n;
    UINT16 val = 0;

    if ( flags.f.def != z80flags::UNDEF )
        switch ( flags.f.seg )
        {
        case z80flags::TEXT:  val = textlen;   break;
        case z80flags::DATA:  val = datalen;   break;
        case z80flags::BSS:   val = bsslen;    break;
        }

    n = find_symbol( sym );
    if ( n )
    {
        if ( n->flags.f.def != z80flags::UNDEF  &&
             flags.f.def != z80flags::UNDEF )
        {
            BAIL(( "symbol %s is being redefined!\n", sym ));
        }

        if ( flags.f.def == z80flags::UNDEF )
            return n;

        n->flags = flags;
        n->value = val;
        return n;
    }

    n = new( sym ) z80symbol;
    n->flags = flags;
    n->value = val;
    n->relocs = NULL;
    n->next = symbol_hash[h];
    symbol_hash[h] = n;
    return n;
}

void
z80file :: write       ( char * file )
{
    z80symbol_ondisk * zso;
    z80reloc_ondisk * zro;
    z80header_ondisk hdr;
    z80reloc * zr;
    z80symbol * s;
    UINT32 pos;
    FILE * f;
    int i;

    f = fopen( file, "w" );
    if ( !f )
        BAIL(( "write cannot open file\n" ));

    UCHAR * symbol_seg = new UCHAR[ MAXSEG ];
    UCHAR * reloc_seg   = new UCHAR[ MAXSEG ];
    int symbol_len = 0;
    int reloc_len = 0;

    // build segments

    for ( i = 0; i < HASH_SIZE; i++ )
    {
        for ( s = symbol_hash[i]; s; s = s->next )
        {
            int reloc_offset;
            zso = (z80symbol_ondisk *)(symbol_seg + symbol_len);
            reloc_offset = symbol_len;
            symbol_len += s->len;

            zso->len = s->len;
            zso->flags = s->flags;
            zso->value.set( s->value );
            strcpy( zso->sym, s->sym );

            for ( zr = s->relocs; zr; zr = zr->next )
            {
                zro = (z80reloc_ondisk *)(reloc_seg + reloc_len);
                reloc_len += sizeof( z80reloc_ondisk );
                zro->symtab_offset.set( reloc_offset );
                zro->offset.set( zr->offset );
                zro->flags = zr->flags;
            }
        }
    }

    // fill in header, and write everything out.

    if ( relocatable )
        hdr.magic.set( Z80_MAGIC_O );
    else
        hdr.magic.set( Z80_MAGIC_E );

    hdr.text_address.set( text_start );
    hdr.data_address.set( data_start );
    hdr.bss_address.set( bss_start );
    hdr.end_address.set( end_addr );

    hdr.text_size.set( textlen );
    hdr.data_size.set( datalen );
    hdr.bss_size.set( bsslen );
    hdr.symtab_size.set( symbol_len );
    hdr.relocs_size.set( reloc_len );

    pos = sizeof( hdr );
    hdr.text_offset.set( pos );
    pos += textlen;
    hdr.data_offset.set( pos );
    pos += datalen;
    hdr.symtab_offset.set( pos );
    pos += symbol_len;
    hdr.relocs_offset.set( pos );

    fwrite( &hdr, sizeof( hdr ), 1, f );
    fwrite( text, textlen, 1, f );
    fwrite( data, datalen, 1, f );
    fwrite( symbol_seg, symbol_len, 1, f );
    fwrite( reloc_seg, reloc_len, 1, f );

    delete[] symbol_seg;
    delete[] reloc_seg;

    fclose( f );
}

void
z80file :: read        ( char * file )
{
    FILE * f = fopen( file, "r" );
    if ( !f )
        BAIL(( "read cannot open file\n" ));

    z80header_ondisk zh;

    if ( fread( &zh, sizeof( zh ), 1, f ) != 1 )
        BAIL(("cannot fread input file header\n"));

    if ( zh.magic.get() == Z80_MAGIC_O )
        relocatable = true;
    else if ( zh.magic.get() == Z80_MAGIC_E )
        relocatable = false;
    else
        BAIL(("bogus Z80 magic in input file\n"));

    textlen = zh.text_size.get();
    datalen = zh.data_size.get();
    bsslen  = zh.bss_size.get();

    if ( textlen > 0 )
    {
        fseek( f, zh.text_offset.get(), SEEK_SET );
        if ( fread( text, textlen, 1, f ) != 1 )
            BAIL(("cannot read text segment\n"));
    }

    if ( datalen > 0 )
    {
        fseek( f, zh.data_offset.get(), SEEK_SET );
        if ( fread( data, datalen, 1, f ) != 1 )
            BAIL(("cannot read data segment\n"));
    }

    text_start = zh.text_address.get();
    data_start = zh.data_address.get();
    bss_start  = zh.bss_address.get();

    UCHAR * symseg = new UCHAR[ MAXSEG ];
    UCHAR * relocseg = new UCHAR[ MAXSEG ];
    UINT16 symseg_size = zh.symtab_size.get();
    UINT16 relocseg_size = zh.relocs_size.get();

    if ( symseg_size > 0 )
    {        
        fseek( f, zh.symtab_offset.get(), SEEK_SET );
        if ( fread( symseg, symseg_size, 1, f ) != 1 )
            BAIL(("cannot read symbol segment\n"));
    }

    if ( relocseg_size > 0 )
    {
        fseek( f, zh.relocs_offset.get(), SEEK_SET );
        if ( fread( relocseg, relocseg_size, 1, f ) != 1 )
            BAIL(("cannot read symbol segment\n"));
    }

    // crack symseg apart

    int i, len;
    for ( i = 0; i < symseg_size; i += len )
    {
        z80symbol_ondisk * zso = (z80symbol_ondisk*)(symseg+i);
        h = hash( zso->sym );
        z80symbol * n = new( zso->sym ) z80symbol;
        n->flags = zso->flags;
        n->value = zso->value.get();
        n->relocs = NULL;
        n->next = symbol_hash[h];
        symbol_hash[h] = n;
        len = zso->len;
    }

    // crack relocseg apart

    for ( i = 0; i < relocseg_size; i += sizeof( z80reloc_ondisk ))
    {
        z80reloc_ondisk * zro = (z80reloc_ondisk *)(relocseg+i);
        z80symbol_ondisk * zso =
            (z80symbol_ondisk*)(symseg+(zro->symtab_offset.get()));
        z80symbol * s = find_symbol( zso->sym );
        if ( !s )
            BAIL(("bogus relocation entry in input file!\n"));
        s->add_reloc( zro->offset.get(), zro->flags );
    }

    delete[] symseg;
    delete[] relocseg;
    fclose( f );
}

// NOTE!  "nf" is not usable after this call! 
//        this function works by first killing the new file
//        and then stealing symbols and relocs out of the carcass.

void
z80file :: link_add( z80file * nf )
{
    if ( !relocatable || !nf->relocatable )
        BAIL(( "cannot link_add a non-relocatable file\n" ));

    if (( textlen + nf->textlen ) > MAXSEG )
        BAIL(( "link_add: text overflow\n" ));

    if (( datalen + nf->datalen ) > MAXSEG )
        BAIL(( "link_add: data overflow\n" ));

    if (( bsslen + nf->bsslen ) > MAXSEG )
        BAIL(( "link_add: bss overflow\n" ));

    int i;
    z80symbol * zs, * mzs, * nzs;
    z80reloc * zr, * nzr;

    // walk the new file's symbol table.
    // if we don't have a given symbol,
    // then steal the pointer out of their file
    // and insert it into our own symtab
    // (and therefore their reloc entries en masse).
    // oh, but we do have to update their reloc entries.
    // if we do have the symbol, then steal
    // their reloc entries one by one, and delete
    // the symbol from their table when done.

    for ( i = 0; i < HASH_SIZE; i++ )
    {
        for ( zs = nf->symbol_hash[i]; zs; zs = nzs )
        {
            nzs = zs->next;
            mzs = find_symbol( zs->sym );

            if ( mzs && mzs->flags.f.def == z80flags::PRIVATE )
                // pretend we didn't find it.
                mzs = NULL;

            if ( mzs )
            {
                if ( mzs->flags.f.def != z80flags::UNDEF  &&
                     zs->flags.f.def != z80flags::UNDEF )
                {
                    BAIL(( "multiply defined symbol '%s'\n", zs->sym ));
                }

                if ( zs->flags.f.def != z80flags::UNDEF )
                {
                    // the new file defined a previously undef'd symbol.
                    // update 'value' and flags.
                    switch ( zs->flags.f.seg )
                    {
                    case z80flags::TEXT:
                        mzs->value = textlen + zs->value;   break;
                    case z80flags::DATA:
                        mzs->value = datalen + zs->value;   break;
                    case z80flags::BSS:
                        mzs->value = bsslen + zs->value;    break;
                    }
                    mzs->flags.f.def = zs->flags.f.def;
                    mzs->flags.f.seg = zs->flags.f.seg;
                }

                for ( zr = zs->relocs; zr; zr = nzr )
                {
                    switch ( zr->flags.f.seg )
                    {
                    case z80flags::TEXT:
                        zr->offset += textlen;   break;
                    case z80flags::DATA:
                        zr->offset += datalen;   break;
                    case z80flags::BSS:
                        zr->offset += bsslen;    break;
                    }
                    nzr = zr->next;
                    zr->next = mzs->relocs;
                    mzs->relocs = zr;
                }

                delete zs;
            }
            else
            {
                // just steal 'zs' and insert it into our
                // own symbol table. update its value if its
                // a definition.
                if ( zs->flags.f.def != z80flags::UNDEF )
                    switch ( zs->flags.f.seg )
                    {
                    case z80flags::TEXT:
                        zs->value += textlen;   break;
                    case z80flags::DATA:          
                        zs->value += datalen;   break;
                    case z80flags::BSS:           
                        zs->value += bsslen;    break;
                    }

                for ( zr = zs->relocs; zr; zr = zr->next )
                {
                    switch ( zr->flags.f.seg )
                    {
                    case z80flags::TEXT:
                        zr->offset += textlen;   break;
                    case z80flags::DATA:
                        zr->offset += datalen;   break;
                    case z80flags::BSS:
                        zr->offset += bsslen;    break;
                    }
                }
                h = hash( zs->sym );
                zs->next = symbol_hash[h];
                symbol_hash[h] = zs;
            }
        }
        nf->symbol_hash[i] = NULL;
    }

    // append text segment

    memcpy( text + textlen, nf->text, nf->textlen );
    textlen += nf->textlen;

    // append data segment

    memcpy( data + datalen, nf->data, nf->datalen );
    datalen += nf->datalen;

    bsslen += nf->bsslen;
}

#define ADD_SYM(sym,segment,val) \
{ \
    s = find_symbol( #sym ); \
    if ( !s ) \
    { \
        s = new( #sym ) z80symbol; \
        s->relocs = NULL; \
        s->next = symbol_hash[h]; \
        symbol_hash[h] = s; \
    } \
    else \
        if ( s->flags.f.def != z80flags::UNDEF ) \
            BAIL(( "'" #sym "' symbol is already defined\n" )); \
    s->flags.f.def = z80flags::PUBLIC; \
    s->flags.f.seg = z80flags::segment; \
    s->flags.f.rel = 0; \
    s->value = val; \
}

void
z80file :: link_final( UINT16 entry )
{
    if ( !relocatable )
        BAIL(( "object file is not a relocatable image!\n" ));

    int i;
    z80symbol * s, * ps, * ns;

    // provide 'start', 'etext', 'edata', 'end' symbols

    ADD_SYM(start,TEXT,0);
    ADD_SYM(etext,TEXT,textlen);
    ADD_SYM(edata,DATA,datalen);
    ADD_SYM(end,BSS,bsslen);

    bool undefs = false;
    // if there are any undefs, it can't be done.
    for ( i = 0; i < HASH_SIZE; i++ )
        for ( ps = NULL, s = symbol_hash[i]; s; ps = s, s = ns )
        {
            ns = s->next;
            if ( s->flags.f.def == z80flags::UNDEF )
            {
                printf( "undefined symbol '%s'\n", s->sym ); 
                undefs = true;
            }
        }

    if ( undefs )
        exit( 1 );

    // rewrite addresses for all symbols, starting at 'entry';
    // first do text, then do data, then bss.

    text_start = entry;
    data_start = text_start + textlen;
    bss_start  = data_start + datalen;
    end_addr   =  bss_start +  bsslen;

#define WALK_SEG(segment,offset) \
    for ( i = 0; i < HASH_SIZE; i++ ) \
        for ( s = symbol_hash[i]; s; s = s->next ) \
            if ( s->flags.f.seg == z80flags::segment ) \
            { \
                UINT32 nv = s->value + offset; \
                if ( nv > MAXSEG ) \
                    BAIL(( "link_final: max segment overflow\n" )); \
                s->value = (UINT16) nv; \
            }

    WALK_SEG( TEXT, text_start );
    WALK_SEG( DATA, data_start );
    WALK_SEG( BSS,  bss_start  );

    // walk symtab, performing all relocations

    for ( i = 0; i < HASH_SIZE; i++ )
    {
        for ( s = symbol_hash[i]; s; s = s->next )
        {
            z80reloc * zr, * nzr;
            for ( zr = s->relocs; zr; zr = nzr )
            {
                UCHAR * addr;
                UINT16 t;
                int diff;
                switch ( zr->flags.f.seg )
                {
                case z80flags::TEXT: addr = text;   break;
                case z80flags::DATA: addr = data;   break;
                case z80flags::BSS:  BAIL(( "cannot relocate in BSS seg\n" ));
                }
                addr += zr->offset;
                UINT16_t * addr16 = (UINT16_t*)addr;
                switch ( zr->flags.f.rel )
                {
                case z80flags::RELATIVE:
                    if ( zr->flags.f.seg != z80flags::TEXT )
                        BAIL(( "cannot relative-relocate non-text sym\n" ));
                    diff = s->value - text_start;
                    diff -= zr->offset;
                    diff -= 1;
                    if ( diff < -128  ||  diff > 127 )
                        BAIL(( "relative offset is outside 7-bit range\n" ));
                    *addr = diff;
                    break;
                case z80flags::RELOC16:
                    addr16->set( addr16->get() + s->value );
                    break;
                case z80flags::RELOC8H:
                    t = s->value + *addr;
                    *addr = ( t >> 8 ) & 0xff;
                    break;
                case z80flags::RELOC8L:
                    t = s->value + *addr;
                    *addr = t & 0xff;
                    break;
                }
                nzr = zr->next;
            }
            s->relocs = NULL;
        }
    }

    relocatable = false;
}

void
z80file :: dump_syms( void )
{
    for ( int i = 0 ; i < HASH_SIZE; i++ )
    {
        for ( z80symbol * s = symbol_hash[i]; s; s = s->next )
        {
            if ( s->flags.f.def == z80flags::UNDEF )
                printf( "     U %s\n", s->sym );
            else
            {
                char type = 'u';
                switch ( s->flags.f.seg )
                {
                case z80flags::TEXT:  type = 'T';  break;
                case z80flags::DATA:  type = 'D';  break;
                case z80flags::BSS:   type = 'B';  break;
                }
                if ( s->flags.f.def == z80flags::PRIVATE )
                    type += 0x20;
                printf( "%04x %c %s\n", s->value, type, s->sym );
            }
        }
    }
}

void
z80file :: dump_relocs( void )
{
    int i;
    z80symbol * s;
    z80reloc * r;

    for ( i = 0 ; i < HASH_SIZE; i++ )
    {
        for ( s = symbol_hash[i]; s; s = s->next )
        {
            for ( r = s->relocs; r; r = r->next )
            {
                char type = 'u';
                switch ( r->flags.f.seg )
                {
                case z80flags::TEXT:  type = 'T';  break;
                case z80flags::DATA:  type = 'D';  break;
                case z80flags::BSS:   type = 'B';  break;
                }
                const char * stype = "<unkn> ";
                switch ( r->flags.f.rel )
                {
                case z80flags::RELATIVE: stype = "RELAT  ";  break;
                case z80flags::RELOC16:  stype = "RELOC16";  break;
                case z80flags::RELOC8H:  stype = "RELOC8H";  break;
                case z80flags::RELOC8L:  stype = "RELOC8L";  break;
                }
                printf( "%04x %c %s %s\n",
                        r->offset, type, stype, s->sym );
            }
        }
    }
}

static void
dump_hex( UCHAR * p, int len )
{
    int pos;
    for ( pos = 0; len > 0; len--, pos++ )
    {
        if ( (pos%16) == 0 )
            printf( "%04x :  ", pos );
        printf( "%02x ", p[pos] );
        if ( (pos%16) == 3 || (pos%16) == 7 || (pos%16) == 11 )
            printf( "  " );
        if ( (pos%16) == 15 )
            printf( "\n" );
    }
    if ( (pos%16) != 0 )
        printf( "\n" );
}

void
z80file :: dump_sgmts( void )
{
    printf( "**** text segment:\n" );
    dump_hex( text, textlen );
    printf( "**** data segment:\n" );
    dump_hex( data, datalen );
}
