#
# Makefile for the CUPL/CORC project
#
VERS=1.9

CDEBUG = -g	# use -O for production, -g for debugging
YFLAGS = -vt	# use -l for production, -vt for debugging
CFLAGS = $(CDEBUG) -Wall -Wextra -std=c11 -Wstrict-prototypes -Wold-style-definition -D_POSIX_SOURCE -DPARSEDEBUG	-DYYDEBUG=1

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
	# Hmmm...this is probably Bison-specific
	awk <tokens.h >toktab.h '/^# define	/ {print $$3 ", \"" $$3 "\","}'

lextest: lexer.c tokens.h tokdump.o
	$(CC) $(CFLAGS) -DMAIN lexer.c tokdump.o -o lextest 

cupl.1: cupl.xml
	xmlto man cupl.xml

cupl.html: cupl.xml
	xmlto html-nochunks cupl.xml

DOCS = README COPYING NEWS control corc.doc cupl.doc cupl.xml
SOURCES = Makefile cupl.[lyh] $(MODULES:.o=.c)
TESTS = test/[abcdefghijklmnopqrstuvwxyz]* test/MAKEREGRESS test/REGRESS test/TESTALL

cupl-$(VERS).tar.gz: $(SOURCES) $(DOCS) cupl.1
	@(cd ..; ln -s cupl cupl-$(VERS))
	@ls $(SOURCES) $(DOCS) $(TESTS) cupl.1 | sed s:^:cupl-$(VERS)/: >MANIFEST
	(cd ..; tar -czf cupl/cupl-$(VERS).tar.gz `cat cupl/MANIFEST`)
	@(cd ..; rm cupl-$(VERS))

dist: cupl-$(VERS).tar.gz

clean:
	rm -f cupl toktab.h tokens.h grammar.c lexer.c lextest y.output 
	rm -f *.o *~ *.1 *.rpm cupl-*.tar.gz *.html MANIFEST

release: cupl-$(VERS).tar.gz cupl.html
	shipper version=$(VERS) | sh -e -x

refresh: cupl.html
	shipper -N -w version=$(VERS) | sh -e -x
