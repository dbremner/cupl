#
# Makefile for the CUPL/CORC project
#

# Note: When the version changes, you also have to change
# the RPM spec file and the lsm.
VERS=1.4

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
	# Hmmm...this is probably Bison-specific
	awk <tokens.h >toktab.h '/^# define	/ {print $$3 ", \"" $$3 "\","}'

lextest: lexer.c tokens.h tokdump.o
	$(CC) $(CFLAGS) -DMAIN lexer.c tokdump.o -o lextest 

cupl.1: cupl.xml
	xmlto man cupl.xml

DOCS = READ.ME corc.doc cupl.doc cupl.xml cupl.lsm
SOURCES = Makefile cupl.[lyh] $(MODULES:.o=.c)

cupl-$(VERS).tar.gz: $(SOURCES) $(DOCS) cupl.1
	@ls $(SOURCES) $(DOCS) cupl.1 | sed s:^:cupl-$(VERS)/: >MANIFEST
	@(cd ..; ln -s cupl cupl-$(VERS))
	(cd ..; tar -czvf cupl/cupl-$(VERS).tar.gz `cat cupl/MANIFEST`)
	@(cd ..; rm cupl-$(VERS))

dist: cupl-$(VERS).tar.gz

RPMROOT=/usr/src/redhat
RPM = rpm
RPMFLAGS = -ba
rpm: dist
	cp cupl-$(VERS).tar.gz $(RPMROOT)/SOURCES;
	cp cupl.spec $(RPMROOT)/SPECS
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) cupl.spec	
	cp $(RPMROOT)/RPMS/`arch|sed 's/i[4-9]86/i386/'`/cupl-$(VERS)*.rpm .
	cp $(RPMROOT)/SRPMS/cupl-$(VERS)*.src.rpm .

clean:
	rm -f cupl *~ *.o toktab.h tokens.h grammar.c lexer.c lextest y.output cupl.tar cupl.tar.gz *.rpm cupl-*.tar.gz

