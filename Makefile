#
# Makefile for the CUPL/CORC project
#

# Note: When the version changes, you also have to change
#  * the name of the containing directory
#  * the RPM spec file
V=1.2

CDEBUG = -g	# use -O for production, -g for debugging
YFLAGS = -vt	# use -l for production, -vt for debugging
CFLAGS = $(CDEBUG) -D_POSIX_SOURCE -DPARSEDEBUG	-DYYDEBUG=1

MODULES = main.o grammar.o lexer.o interpret.o tokdump.o execute.o monitor.o
cupl: $(MODULES)
	$(CC) $(MODULES) -lm -o cupl

# You can use either lex or flex
#LEX = lex
LEX = flex

# You can use either bison or yacc
#YACC = yacc
YACC = bison -y

lexer.c: cupl.l
	$(LEX) $(LFLAGS) cupl.l
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

cupl.tar: 
	(cd ..; tar -cvf cupl-$(V)/cupl.tar `cat cupl-$(V)/MANIFEST | sed "/\(^\| \)/s// cupl-$(V)\//g"`)
cupl.tar.gz: cupl.tar
	gzip -f cupl.tar

clean:
	rm -f cupl *~ *.o toktab.h tokens.h grammar.c lexer.c lextest y.output cupl.tar cupl.tar.gz

rpm: cupl.tar.gz
	cp cupl.tar.gz /usr/src/SOURCES/cupl-$(V).tar.gz
	cp cupl.spec /usr/src/SPECS/cupl-$(V)-1.spec
	rpm -ba cupl-$(V)-1.spec
