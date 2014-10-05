
FLEX= /usr/local/bin/flex++
YACC= /usr/local/bin/bison
OBJS= c-parse.tab.o c-token.o types.o node.o main.o
PROG= t
OPTS=  -c -g3 -Wno-deprecated

all: $(PROG)

$(PROG): $(OBJS)
	g++ -o $(PROG) $(OBJS)

.c.o:
	gcc $(OPTS) $*.c

.C.o:
	g++ $(OPTS) $*.C

c-token.C: c-token.l
	$(FLEX) -oc-token.C c-token.l

lex.yy.o: lex.yy.C
	g++ $(OPTS) lex.yy.C

c-parse.tab.C: c-parse.Y
	$(YACC) -dv c-parse.Y

c-parse.tab.o: c-parse.tab.C
	g++ $(OPTS) c-parse.tab.C

clean:
	rm -f c-token.C c-parse.tab.[CH] c-parse.output *.o $(PROG)
	rm -f *~ \#*

depend: c-token.C c-parse.tab.C
	awk 'BEGIN { copy=1; } { if ( copy==1 ) print; } /^# DEPENDENCIES/ { copy=0; }' Makefile > x
	gcc -Wno-deprecated -M *.c *.C >> x
	mv x Makefile
sample.o: sample.c
c-parse.tab.o: c-parse.tab.C /usr/include/stdio.h \
  /usr/include/sys/cdefs.h /usr/include/sys/_null.h \
  /usr/include/sys/_types.h /usr/include/machine/_types.h \
  /usr/include/stdlib.h types.H node.H dll2.H
c-token.o: c-token.C /usr/include/stdlib.h /usr/include/sys/cdefs.h \
  /usr/include/sys/_null.h /usr/include/sys/_types.h \
  /usr/include/machine/_types.h /usr/include/c++/3.3/FlexLexer.h \
  /usr/include/c++/3.3/backward/iostream.h \
  /usr/include/c++/3.3/backward/backward_warning.h \
  /usr/include/c++/3.3/iostream /usr/include/c++/3.3/bits/c++config.h \
  /usr/include/c++/3.3/bits/os_defines.h /usr/include/c++/3.3/ostream \
  /usr/include/c++/3.3/ios /usr/include/c++/3.3/iosfwd \
  /usr/include/c++/3.3/bits/c++locale.h /usr/include/c++/3.3/clocale \
  /usr/include/locale.h /usr/include/c++/3.3/cctype /usr/include/ctype.h \
  /usr/include/runetype.h /usr/include/c++/3.3/bits/stringfwd.h \
  /usr/include/c++/3.3/bits/fpos.h /usr/include/c++/3.3/bits/c++io.h \
  /usr/include/c++/3.3/cstdio /usr/include/c++/3.3/cstddef \
  /usr/include/stddef.h /usr/include/stdio.h \
  /usr/include/c++/3.3/bits/gthr.h \
  /usr/include/c++/3.3/bits/gthr-default.h /usr/include/pthread.h \
  /usr/include/sys/types.h /usr/include/machine/endian.h \
  /usr/include/sys/select.h /usr/include/sys/_sigset.h \
  /usr/include/sys/_timeval.h /usr/include/sys/timespec.h \
  /usr/include/sys/time.h /usr/include/time.h /usr/include/sys/signal.h \
  /usr/include/machine/signal.h /usr/include/machine/trap.h \
  /usr/include/limits.h /usr/include/sys/limits.h \
  /usr/include/machine/_limits.h /usr/include/sys/syslimits.h \
  /usr/include/sched.h /usr/include/unistd.h /usr/include/sys/unistd.h \
  /usr/include/c++/3.3/cwchar /usr/include/c++/3.3/ctime \
  /usr/include/wchar.h /usr/include/c++/3.3/bits/functexcept.h \
  /usr/include/c++/3.3/exception_defines.h /usr/include/c++/3.3/exception \
  /usr/include/c++/3.3/bits/char_traits.h /usr/include/c++/3.3/cstring \
  /usr/include/string.h /usr/include/strings.h \
  /usr/include/c++/3.3/bits/localefwd.h \
  /usr/include/c++/3.3/bits/ios_base.h \
  /usr/include/c++/3.3/bits/atomicity.h \
  /usr/include/c++/3.3/bits/locale_classes.h /usr/include/c++/3.3/string \
  /usr/include/c++/3.3/memory /usr/include/c++/3.3/bits/stl_algobase.h \
  /usr/include/c++/3.3/climits /usr/include/c++/3.3/cstdlib \
  /usr/include/c++/3.3/new /usr/include/c++/3.3/bits/stl_pair.h \
  /usr/include/c++/3.3/bits/type_traits.h \
  /usr/include/c++/3.3/bits/stl_iterator_base_types.h \
  /usr/include/c++/3.3/bits/stl_iterator_base_funcs.h \
  /usr/include/c++/3.3/bits/concept_check.h \
  /usr/include/c++/3.3/bits/stl_iterator.h \
  /usr/include/c++/3.3/bits/stl_alloc.h \
  /usr/include/c++/3.3/bits/stl_threads.h \
  /usr/include/c++/3.3/bits/stl_construct.h \
  /usr/include/c++/3.3/bits/stl_uninitialized.h \
  /usr/include/c++/3.3/bits/stl_raw_storage_iter.h \
  /usr/include/c++/3.3/bits/stl_function.h \
  /usr/include/c++/3.3/bits/basic_string.h /usr/include/c++/3.3/algorithm \
  /usr/include/c++/3.3/bits/stl_algo.h \
  /usr/include/c++/3.3/bits/stl_heap.h \
  /usr/include/c++/3.3/bits/stl_tempbuf.h \
  /usr/include/c++/3.3/bits/basic_string.tcc \
  /usr/include/c++/3.3/streambuf /usr/include/c++/3.3/bits/streambuf.tcc \
  /usr/include/c++/3.3/bits/basic_ios.h \
  /usr/include/c++/3.3/bits/streambuf_iterator.h \
  /usr/include/c++/3.3/bits/locale_facets.h /usr/include/c++/3.3/cwctype \
  /usr/include/wctype.h /usr/include/c++/3.3/bits/ctype_base.h \
  /usr/include/c++/3.3/bits/ctype_inline.h \
  /usr/include/c++/3.3/bits/codecvt.h \
  /usr/include/c++/3.3/bits/codecvt_specializations.h \
  /usr/include/c++/3.3/bits/time_members.h \
  /usr/include/c++/3.3/bits/messages_members.h \
  /usr/include/c++/3.3/bits/basic_ios.tcc \
  /usr/include/c++/3.3/bits/ostream.tcc /usr/include/c++/3.3/locale \
  /usr/include/c++/3.3/bits/locale_facets.tcc /usr/include/c++/3.3/cerrno \
  /usr/include/errno.h /usr/include/c++/3.3/cmath /usr/include/math.h \
  /usr/include/c++/3.3/bits/cmath.tcc /usr/include/c++/3.3/limits \
  /usr/include/c++/3.3/typeinfo /usr/include/c++/3.3/istream \
  /usr/include/c++/3.3/bits/istream.tcc node.H dll2.H c-parse.tab.H \
  types.H
main.o: main.C /usr/include/c++/3.3/iostream \
  /usr/include/c++/3.3/bits/c++config.h \
  /usr/include/c++/3.3/bits/os_defines.h /usr/include/c++/3.3/ostream \
  /usr/include/c++/3.3/ios /usr/include/c++/3.3/iosfwd \
  /usr/include/c++/3.3/bits/c++locale.h /usr/include/c++/3.3/clocale \
  /usr/include/locale.h /usr/include/sys/_null.h /usr/include/sys/cdefs.h \
  /usr/include/c++/3.3/cctype /usr/include/ctype.h \
  /usr/include/sys/_types.h /usr/include/machine/_types.h \
  /usr/include/runetype.h /usr/include/c++/3.3/bits/stringfwd.h \
  /usr/include/c++/3.3/bits/fpos.h /usr/include/c++/3.3/bits/c++io.h \
  /usr/include/c++/3.3/cstdio /usr/include/c++/3.3/cstddef \
  /usr/include/stddef.h /usr/include/stdio.h \
  /usr/include/c++/3.3/bits/gthr.h \
  /usr/include/c++/3.3/bits/gthr-default.h /usr/include/pthread.h \
  /usr/include/sys/types.h /usr/include/machine/endian.h \
  /usr/include/sys/select.h /usr/include/sys/_sigset.h \
  /usr/include/sys/_timeval.h /usr/include/sys/timespec.h \
  /usr/include/sys/time.h /usr/include/time.h /usr/include/sys/signal.h \
  /usr/include/machine/signal.h /usr/include/machine/trap.h \
  /usr/include/limits.h /usr/include/sys/limits.h \
  /usr/include/machine/_limits.h /usr/include/sys/syslimits.h \
  /usr/include/sched.h /usr/include/unistd.h /usr/include/sys/unistd.h \
  /usr/include/c++/3.3/cwchar /usr/include/c++/3.3/ctime \
  /usr/include/wchar.h /usr/include/c++/3.3/bits/functexcept.h \
  /usr/include/c++/3.3/exception_defines.h /usr/include/c++/3.3/exception \
  /usr/include/c++/3.3/bits/char_traits.h /usr/include/c++/3.3/cstring \
  /usr/include/string.h /usr/include/strings.h \
  /usr/include/c++/3.3/bits/localefwd.h \
  /usr/include/c++/3.3/bits/ios_base.h \
  /usr/include/c++/3.3/bits/atomicity.h \
  /usr/include/c++/3.3/bits/locale_classes.h /usr/include/c++/3.3/string \
  /usr/include/c++/3.3/memory /usr/include/c++/3.3/bits/stl_algobase.h \
  /usr/include/c++/3.3/climits /usr/include/c++/3.3/cstdlib \
  /usr/include/stdlib.h /usr/include/c++/3.3/new \
  /usr/include/c++/3.3/bits/stl_pair.h \
  /usr/include/c++/3.3/bits/type_traits.h \
  /usr/include/c++/3.3/bits/stl_iterator_base_types.h \
  /usr/include/c++/3.3/bits/stl_iterator_base_funcs.h \
  /usr/include/c++/3.3/bits/concept_check.h \
  /usr/include/c++/3.3/bits/stl_iterator.h \
  /usr/include/c++/3.3/bits/stl_alloc.h \
  /usr/include/c++/3.3/bits/stl_threads.h \
  /usr/include/c++/3.3/bits/stl_construct.h \
  /usr/include/c++/3.3/bits/stl_uninitialized.h \
  /usr/include/c++/3.3/bits/stl_raw_storage_iter.h \
  /usr/include/c++/3.3/bits/stl_function.h \
  /usr/include/c++/3.3/bits/basic_string.h /usr/include/c++/3.3/algorithm \
  /usr/include/c++/3.3/bits/stl_algo.h \
  /usr/include/c++/3.3/bits/stl_heap.h \
  /usr/include/c++/3.3/bits/stl_tempbuf.h \
  /usr/include/c++/3.3/bits/basic_string.tcc \
  /usr/include/c++/3.3/streambuf /usr/include/c++/3.3/bits/streambuf.tcc \
  /usr/include/c++/3.3/bits/basic_ios.h \
  /usr/include/c++/3.3/bits/streambuf_iterator.h \
  /usr/include/c++/3.3/bits/locale_facets.h /usr/include/c++/3.3/cwctype \
  /usr/include/wctype.h /usr/include/c++/3.3/bits/ctype_base.h \
  /usr/include/c++/3.3/bits/ctype_inline.h \
  /usr/include/c++/3.3/bits/codecvt.h \
  /usr/include/c++/3.3/bits/codecvt_specializations.h \
  /usr/include/c++/3.3/bits/time_members.h \
  /usr/include/c++/3.3/bits/messages_members.h \
  /usr/include/c++/3.3/bits/basic_ios.tcc \
  /usr/include/c++/3.3/bits/ostream.tcc /usr/include/c++/3.3/locale \
  /usr/include/c++/3.3/bits/locale_facets.tcc /usr/include/c++/3.3/cerrno \
  /usr/include/errno.h /usr/include/c++/3.3/cmath /usr/include/math.h \
  /usr/include/c++/3.3/bits/cmath.tcc /usr/include/c++/3.3/limits \
  /usr/include/c++/3.3/typeinfo /usr/include/c++/3.3/istream \
  /usr/include/c++/3.3/bits/istream.tcc /usr/include/c++/3.3/fstream \
  /usr/include/c++/3.3/bits/basic_file.h \
  /usr/include/c++/3.3/bits/fstream.tcc /usr/include/c++/3.3/FlexLexer.h \
  /usr/include/c++/3.3/backward/iostream.h \
  /usr/include/c++/3.3/backward/backward_warning.h node.H dll2.H
node.o: node.C /usr/include/stdio.h /usr/include/sys/cdefs.h \
  /usr/include/sys/_null.h /usr/include/sys/_types.h \
  /usr/include/machine/_types.h node.H dll2.H
types.o: types.C /usr/include/stdio.h /usr/include/sys/cdefs.h \
  /usr/include/sys/_null.h /usr/include/sys/_types.h \
  /usr/include/machine/_types.h /usr/include/stdlib.h \
  /usr/include/string.h /usr/include/strings.h types.H
