#
# Makefile for the CUPL project
#
CUPLDIR = .	# Not presently used -- will be if we implement the compiler

YFLAGS = -vlt
CFLAGS = -g -D_POSIX_SOURCE -DYYDEBUG=1 -DPARSEDEBUG

SOURCES = cupl.[ylh] tokdump.c main.c interpret.c

cupl: main.o grammar.o lexer.o interpret.o tokdump.o
	$(CC) main.o grammar.o lexer.o interpret.o tokdump.o -o cupl

lexer.c: cupl.l
	lex $(LFLAGS) cupl.l
	mv lex.yy.c lexer.c

grammar.c tokens.h y.output: cupl.y
	$(YACC) $(YFLAGS) -d cupl.y
	@echo "expect: conflicts: 5 shift/reduce"
	mv y.tab.c grammar.c
	mv y.tab.h tokens.h

grammar.o: grammar.c cupl.h
tokdump.o: tokdump.c toktab.h
lexer.o: lexer.c tokens.h cupl.h
interpret.o: interpret.c tokens.h cupl.h

toktab.h: tokens.h
	awk <tokens.h >toktab.h '/^#/ {print $$3 ", \"" $$3 "\","}'

lextest: lexer.c tokens.h tokdump.o
	$(CC) $(CFLAGS) -DMAIN lexer.c tokdump.o -o lextest 

clean:
	rm -f cupl *~ *.o toktab.h tokens.h grammar.c lexer.c lextest y.output
