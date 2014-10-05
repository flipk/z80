
//xxx
// how do we handle a construct of the form   &array[offset]   ?
//  it is going to be an addrof containing an arrayref
//   containing the array var and offset value.
//  it should end up as plus containing addrof and offset, 
//  where addrof contains the array var.
// how to get from one to the other?

// NOTE

// the structures used in this file have a great deal of pointer
// sharing.  pointers are shared like crazy in tangled nests.

#include "dll2.H"

enum c_node_base_types {
    NODE_UNUSED            = 0x0000000,
    NODE_CONST             = 0x94142d6,
    NODE_STRINGCONST       = 0x01eb274,
    NODE_SIMPLE_TYPE       = 0xd686ec6,
    NODE_POINTER_TYPE      = 0x9d2f28b,   /* note similar to array type! */
    NODE_AGGREGATE_TYPE    = 0x6ee2898,
    NODE_STORAGE           = 0x8502043,
    NODE_INSTRS            = 0xfb2875b,
    NODE_INSTR_VARREF      = 0x97acee6, /* pointer to storage */
    NODE_INSTR_ARRAYREF    = 0x6487aee, /* storage, and offset */
    NODE_INSTR_PTRDEREF    = 0xee25026, /* storage */
    NODE_INSTR_ADDROF      = 0xb4749f6, /* xxx value? */
    NODE_INSTR_PLUS        = 0x0d2620e, /* two values */
};

// base type for a parse node.
// every derived type must have a static const int TYPE = xxx
// from the enum c_node_base_types

struct c_node_base {
public:
    int type;
    int size;
    LListLinks<c_node_base>  links[LListHashLruLinks];    
    char * name;
    c_node_base( int _type, int _size ) {
        type = _type;
        size = _size;
    }
    template <class T>
    bool convert( T ** t ) {
        if ( type == T::TYPE )
        {
            *t = (T*)this;
            return true;
        }
        else
        {
            return false;
        }
    }
};

// this node defines a constant value; regardless of the actual type,
// it is stored here as a 32-bit unsigned quantity.  numeric constants
// are assumed to be the largest integer type -- assignments to smaller
// types will truncate automatically.

struct c_node_const : public c_node_base {
    static const int TYPE = NODE_CONST;
    unsigned int value;
    c_node_const( void ) :
        c_node_base( TYPE, sizeof( *this )) {}
};

// this node defines a constant string value.

struct c_node_stringconst : public c_node_base {
    static const int TYPE = NODE_STRINGCONST;
    char * string;
    c_node_stringconst( void ) :
        c_node_base( TYPE, sizeof( *this )) {}
};

// this node defines the parameters of a basic type.
// the 'name' field is the name of the type itself.

struct c_node_simpletype : public c_node_base {
    static const int TYPE = NODE_SIMPLE_TYPE;
    int bitwidth;    // zero means type 'void'; rarely found alone
    bool is_signed;
    c_node_simpletype( void ) :
        c_node_base( TYPE, sizeof( *this )) {}
};

// this node defines a pointer to a type, either simple or aggregate.
// 'deref' indicates what type this type points to.
// in this case, 'name' will be of the form
// "ptr-to-%s" where %s is deref->name.
// note that an array type is also defined using this structure.
// the only difference is the 'dimension' value. 
// dimension==-1 means it is a true pointer type.
// dimension==0  means it is an array type whose size is unknown.
// dimension >0  means it is an array type with known size.

struct c_node_pointertype : public c_node_base {
    static const int TYPE = NODE_POINTER_TYPE;
    int dimension;
    c_node_base * deref;
    c_node_pointertype( void ) :
        c_node_base( TYPE, sizeof( *this )) {}
};

// this node defines the parameters of an aggregate type (struct).
// the 'name' field is the name of the type itself.
// 'elements' is an array of pointers to NODE_STORAGE nodes.

struct c_node_aggregatetype : public c_node_base {
    static const int TYPE = NODE_AGGREGATE_TYPE;
    int bytewidth;
    int num_elements;
    c_node_base ** elements;
//xxx
    c_node_aggregatetype( void ) :
        c_node_base( TYPE, sizeof( *this )) {}
};

// this node defines some storage.  the 'name' is the name of the
// storage from the program's point of view; 'mytype' points to
// an actual 'type' definition, either a simpletype or an aggregatetype.
// note that the thing 'mytype' points to is probably on some other
// list of types elsewhere, so the pointer is shared.
// 'is_static' describes whether global storage variables belong
// in the exported symbol table or not.  this field is ignored
// when this node is used inside a struct def.

struct c_node_storage : public c_node_base {
    static const int TYPE = NODE_STORAGE;
    bool is_static;
    c_node_base * mytype;
    c_node_storage( void ) :
        c_node_base( TYPE, sizeof( *this )) {}
};

// this node defines a stream of instructions to perform.
// it contains a list of node_storage elements which are the 
// private local variables necessary for that instruction stream;
// all operations are some derived type of c_node_instruction.
// many operations on the instructions list have pointers which point
// into the local_storage list -- thus those pointers are shared.
// 'name' is the name of the function.

// this structure actually represents a 'context' which in c can
// be nested.  since more than one context can exist at any given
// time, the parser maintains a 'stack' of them, which gets pushed
// when a block is started in the parser, and popped when its complete.
// any varrefs in a piece of c code will search the context stack
// top-down for the proper reference.  thus the pointer sharing ends
// up being quite perverse when the c code has nested contexts.

struct c_node_instr_stream : public c_node_base {
    static const int TYPE = NODE_INSTRS;
    LListHashLru<c_node_base>   local_storage;
    LList <c_node_base,0>  instructions;
    bool is_static;
    c_node_instr_stream( void ) :
        local_storage( 30 ),
        c_node_base( TYPE, sizeof( *this )) {}
};

// we don't define the TYPE because there isn't just one
// for this class. all instrs use this object -- but each
// instr has a different use for the 'args'.  
// if this instr outputs a value, 'mytype' indicates what
// type the value of this output is.  (tricky, because for
// instance if its a pointer deref, then the type of the
// deref node is the same as the value being dereferenced,
// minus one level of NODE_POINTER_TYPE indirection.)

struct c_node_instr : public c_node_base {
//    static const int TYPE;
    c_node_base * mytype;
    c_node_base * args[3];
    c_node_instr( int _type ) :
        c_node_base( _type, sizeof( *this )) {}
};

struct c_file {
    static const int typehashsize     = 100;
    static const int storagehashsize  = 100;
    static const int functionhashsize = 20;

    // this list contains all type definitions --
    // each element should be either a simple type or an aggregate type.
    LListHashLru<c_node_base>   types;

    // this list contains all global variables.
    // each element should be of type node_storage.
    LListHashLru<c_node_base>   global_storage;

    // each element of this list should be an instruction_stream.
    LListHashLru<c_node_base>   functions;

    c_file( void ) : 
        types( typehashsize ),
        global_storage( storagehashsize ),
        functions( functionhashsize ) 
        {
        }
};
