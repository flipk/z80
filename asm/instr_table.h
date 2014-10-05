
#include "instr_enums.h"

enum argument_types {

    A_NUM = 1, A_DEREF_REG, A_DEREF_NUM,
    A_DEREF_IX_PLUS, A_DEREF_IY_PLUS,
    A_FLAG, A_REG, A_CONST, A_REL,
    A_IDENT_A, A_IDENT_H, A_IDENT_L

};

struct instruction_type {
    int mn;
    int arg1;
    int type1;
    int arg2;
    int type2;
};

struct identifier {
    enum argument_types type;
    char * id;
};
struct typeval {
    enum argument_types type;
    int val;
};
struct operand {
    enum argument_types type;
    union {
        int num;
        enum register_types reg;
        enum flag_types flag;
    } val;
    struct identifier id;
};

struct compiled_instruction {
    struct compiled_instruction * next;
    int oplen;
    unsigned char op[ 8 ];
    char * mn;
    enum instruction_types mnid;
    struct operand oper1;
    struct operand oper2;
};

extern struct instruction_type * instruction_tables[];
