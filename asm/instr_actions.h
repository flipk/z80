
void compile_instr_list( void );

int is_register( char * str, int len );
int is_flag( char * str, int len );
int is_instruction( char * str, int len );

extern struct compiled_instruction cur;
struct compiled_instruction * resolve_instruction( void );
