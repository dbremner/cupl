/*****************************************************************************

NAME
   main.c -- main sequence of the CUPL compiler

SYNOPSIS
   cupl [-v] [file...]

DESCRIPTION
   The main sequence of the Cornell University Programming Language compiler.
All the real work is done by yyparse. May set globals verbose and yydebug.

*****************************************************************************/
/*LINTLIBRARY*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "cupl.h"

extern FILE *yyin;		/* the program text file descriptor */
extern int yylineno;		/* the current source line count */
extern int yydebug;		/* enable YACC instrumentation? */

#define CANTOPN	"can't open file %s\n"
#define USAGE	"usage: cupl [-v nnn]\n"

int verbose;	/* verbosity level of the interpreter */

static int execfile(const char *file)
/* translate a CUPL file in the current directory */
{
    if (file == (char *)NULL)
	yyin = stdin;
    else
    {
	if ((yyin = fopen(file, "r")) == (FILE *)NULL)
	{
	    (void) fprintf(stderr, CANTOPN, file);
	    return(1);
	}
    }

    yyparse();		/* build and interpret the parse tree */

    if (file)
	(void) fclose(yyin);

    return(0);
}

main(argc, argv)
int	argc;
char	*argv[];
{
    extern int	    optind;		/* getopt() sets this */
    extern char	    *optarg;		/* and this */
    extern char	    *getenv();
    int	c;

    while ((c = getopt(argc, argv, "v:")) != EOF)
	switch (c)
	{
	case 'v':
	    verbose = atoi(optarg);
	    if (strchr(optarg, 'y'))
		yydebug = 1;
	    break;

	default:
	    (void) fprintf(stderr, USAGE);
	    break;
	}

    if (optind == argc)
	return(execfile((char *)NULL));
    else
	for (; optind < argc; optind++)
	    if (execfile(argv[optind]))
		break;
    return(0);
}

/* main.c ends here */
