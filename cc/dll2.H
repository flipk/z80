
// NOTE THAT THESE CLASSES DO *NO* MEMORY MANAGEMENT.
// IF A LIST IS DELETED, THE ITEMS THAT WERE ON THAT LIST
// ARE *NOT* FREED.  IF AN ITEM IS REMOVED FROM A LIST IT
// IS ALSO NOT FREED.

template <class T, int instance> class LList;

template <class T>
class LListLinks
{
private:
    friend class LList<T,0>;   // hack! hack! hack! --bill the cat
    friend class LList<T,1>;
    friend class LList<T,2>;
    friend class LList<T,3>;
    T * next;
    T * prev;
    bool onlist;
public:
    LListLinks( void ) { next = prev = NULL; onlist = false; }
};

// requirement :  class "T" requires a member
//    LListLinks<T>  links[]
// which this class can access (public in "T")
// its ok to make the LListLinks public in the class
// because all of its contents are completely private
// except to its friends in LList.

template <class T, int instance>
class LList
{
    T * head;
    T * tail;
    int cnt;
public:
    LList( void ) { head = tail = NULL;  cnt = 0; }
    void add( T * x )
        {
            if ( x->links[instance].onlist )
            {
                printf( "ERROR LLIST ENTRY ALREADY ON LIST!\n" );
            }
            else
            {
                x->links[instance].onlist = true;
                LListLinks<T> * ll = & x->links[instance];
                ll->next = NULL;
                ll->prev = tail;
                if ( head )
                    tail->links[instance].next = x;
                else
                    head = x;
                tail = x;
                cnt++;
            }
        }
    void del( T * x )
        {
            if ( x->links[instance].onlist )
            {
                x->links[instance].onlist = false;
                LListLinks<T> * ll = & x->links[instance];
                if ( ll->next )
                    ll->next->links[instance].prev = ll->prev;
                else
                    tail = ll->prev;
                if ( ll->prev )
                    ll->prev->links[instance].next = ll->next;
                else
                    head = ll->next;
                ll->next = ll->prev = NULL;
                cnt--;
            }
            else
            {
                printf( "ERROR LLIST ENTRY NOT ON A LIST\n" );
            }
        }
    int get_cnt( void ) { return cnt; }
    T * get_head( void ) { return head; }
    T * get_tail( void ) { return tail; }
    T * get_next( T * x ) { return x->links[instance].next; }
    T * get_prev( T * x ) { return x->links[instance].prev; }
};

// requirement:    type "T" needs the following lines:
//    char * name
//   LListLinks<T>  links[LListHashLruLinks];

#define LListHashLruLinks 3

template <class T>
class LListHashLru
{
    // two lists : 0 is a linked list of all items;
    //             1 is a linked hash list of hash items;
    //             2 is an LRU. 
    // in the lru, 'add' adds to the tail.  thus 'head' is the
    // oldest item in the lru.
    LList <T,0>  all;
    int hash_size;
    LList <T,1>  * hash;
    LList <T,2>  lru;
public:
    int calc_hash( char * name ) {
        int r = 0; int s = 10; char * c;
        for ( c = name; *c; c++ )
        {
            r += (*c << s);
            s = (s==0)?10:s-1;
        }
        return r % hash_size;
    }
    LListHashLru( int _hash_size ) {
        hash_size = _hash_size;
        hash = new LList<T,1>[ hash_size ];
    }
    ~LListHashLru( void ) {
        T * p, * np;
        for ( np = 0, p = all.get_head(); 
              p;
              p = np )
        {
            np = all.get_next( p );
            all.del( p );
        }
    }
    void add( T * x ) {
        int h = calc_hash( x->name );
        all.add( x );
        lru.add( x );
        hash[h].add( x );
    }
    void del( T * x ) {
        int h = calc_hash( x->name );
        all.dell( x );
        lru.del( x );
        hash[h].del( x );
    }
    T * find( char * name ) {
        int h = calc_hash( name );
        for ( T * r = hash[h].get_head();
              r;
              r = hash[h].get_next( r ))
        {
            if ( strcmp( name, r->name ) == 0 )
                return r;
        }
    }
    void get_cnt( void ) { return all.get_cnt(); }
    T * get_oldest( void ) { return lru.get_head(); }
    void promote( T * x ) { lru.del( x ); lru.add( x ); }
    // "walking" a list with this class always walks
    // in the order of the 'all' list.
    T * get_head( void ) { return all.get_head(); }
    T * get_tail( void ) { return all.get_tail(); }
    T * get_next( T * x ) { return all.get_next( x ); }
    T * get_prev( T * x ) { return all.get_prev( x ); }
};
