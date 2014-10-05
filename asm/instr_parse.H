
bool  is_empty_line( char * line );
bool  parse_special( z80file * object, char * line );
bool  consume_label( z80file * object, char * line );
bool  parse_instruction( z80file * object, char * instr );
