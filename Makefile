#
# Makefile for the CUPL project
#
CDEBUG = -g

YFLAGS = -l	# add -vt for debugging
CFLAGS = $(CDEBUG) -D_POSIX_SOURCE -DPARSEDEBUG	#  -DYYDEBUG=1

SOURCES = cupl.[ylh] tokdump.c main.c interpret.c execute.c monitor.c nodetype.h

MODULES = main.o grammar.o lexer.o interpret.o tokdump.o execute.o monitor.o
cupl: $(MODULES)
	$(CC) $(MODULES) -lm -o cupl

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
interpret.o: interpret.c tokens.h cupl.h
execute.o: execute.c tokens.h cupl.h
monitor.o: monitor.c tokens.h cupl.h

toktab.h: tokens.h
	awk <tokens.h >toktab.h '/^#/ {print $$3 ", \"" $$3 "\","}'

lextest: lexer.c tokens.h tokdump.o
	$(CC) $(CFLAGS) -DMAIN lexer.c tokdump.o -o lextest 

clean:
	rm -f cupl *~ *.o toktab.h tokens.h grammar.c lexer.c lextest y.output
