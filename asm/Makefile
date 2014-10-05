
PROG=	z80asm
HDRS=	instr_table.h instr_actions.h instr_parse.H z80file.H
SRCS= 	instr_table.c instr_actions.c instr_parse.C z80file.C main.C
OBJS= 	instr_enums.o y.tab.o lex.yy.o \
        instr_table.o instr_actions.o instr_parse.o z80file.o main.o

all: mkbin $(PROG) z80nm z80link test

mkbin:
	@if [ ! -d bin ] ; then mkdir bin ; fi

$(PROG): $(OBJS) $(LIBS)
	g++ $(OBJS) $(LIBS) -o $(PROG)
	cp $(PROG) bin

z80nm: namelist.C z80file.o
	g++ namelist.C z80file.o -o z80nm
	cp z80nm bin

z80link: linker.C z80file.o
	g++ linker.C z80file.o -o z80link
	cp z80link bin

instr_enums.h: instr_enums.list
	./enums_to_h instr_enums.list instr_enums.h instr_enums.c

instr_enums.c: instr_enums.h

y.tab.h: parse.y
	yacc -dv parse.y

y.tab.c: y.tab.h

lex.yy.c: token.l
	lex token.l

lex.yy.o: lex.yy.c

.c.o:
	gcc -g3 -c $*.c

.C.o:
	g++ -g3 -c $*.C

test:   z3

z3: z1.o z2.o
	bin/z80link z1.o z2.o -o z3 -e 0x200
	bin/z80nm z3

z1.o: z1.S

z2.o: z2.S

.S.o:
	cpp $*.S > $*.s
	bin/z80asm $*.s $*.o
	rm -f $*.s

clean:
	rm -f $(PROG) $(OBJS) instr_enums.h instr_enums.c
	rm -f lex.yy.c y.*
	rm -f z*.s z*.o z3 z80nm z80link
	rm -rf bin

depend: instr_enums.h
	awk 'BEGIN { matches=0; } { if ( matches < 2 ) { print; }} /DEPENDENCIES APPENDED/ { matches++; } ' Makefile > xmakefile
	gcc -M $(SRCS) >> xmakefile
	mv xmakefile Makefile
	make clean

# DEPENDENCIES APPENDED
