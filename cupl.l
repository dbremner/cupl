%{
/* cupl.l -- lexical analyzer for the CUPL language */
/* SPDX-License-Identifier: BSD-2-clause */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include "tokens.h"
#include "cupl.h"

extern char *strdup(const char *s);

lvar *idlist;
bool corc;

static node *intern_number(char *);
static node *intern_identifier(char *);
static node *intern_string(char *);

#ifdef FLEX_SCANNER
int yylineno;
#endif /* FLEX_SCANNER */
%}

D	[0-9][0-9]*
N	[0-9.]+

%%
COMMENT.*\n	;
NOTE.*\n	{corc = true;}

ABS		{return(ABS);}
ALL		{return(ALL);}
ALLOCATE	{return(ALLOCATE);}
AND		{return(AND);}
ATAN		{return(ATAN);}
BEGIN		{corc = true; return(BLOCK);}
BLOCK		{return(BLOCK);}
BY		{return(BY);}
COMMENT		{return(COMMENT);}
COS		{return(COS);}
DEC		{corc = true; return(DEC);}
DECREASE	{corc = true; return(DEC);}
DET		{return(DET);}
DOT		{return(DOT);}
ELSE		{return(ELSE);}
END		{return(END);}
EQL		{corc = true; return('=');}
EXP		{return(EXP);}
FLOOR		{return(FLOOR);}
FOR		{return(FOR);}
GE		{return(GE);}
GEQ		{corc = true; return(GE);}
GO		{return(GO);}
GT		{return(GT);}
GTR		{corc = true; return(GT);}
IDN		{return(IDN);}
IF		{return(IF);}
INC		{corc = true; return(INC);}
INCREASE	{corc = true; return(INC);}
INT		{corc = true; return(FLOOR);}
INV		{return(INV);}
LE		{return(LE);}
LEQ		{corc = true; return(LE);}
LET		{return(LET);}
LN		{return(LN);}
LOG		{return(LOG);}
LSS		{corc = true; return(LE);}
LT		{return(LT);}
MAX		{return(MAX);}
MIN		{return(MIN);}
NE		{return(NE);}
NEQ		{corc = true; return(NE);}
OR		{return(OR);}
PERFORM		{return(PERFORM);}
POSMAX		{return(POSMAX);}
POSMIN		{return(POSMIN);}
RAND		{return(RAND);}
READ		{return(READ);}
REPEAT		{corc = true; return(PERFORM);}
SGM		{return(SGM);}
SIN		{return(SIN);}
SQRT		{return(SQRT);}
STOP		{return(STOP);}
THEN		{return(THEN);}
TIMES		{return(TIMES);}
TITLE.*\n	{corc = true; yylval.node = intern_string(yytext + 5); return(TITLE);}
TO		{return(TO);}
TRC		{return(TRC);}
TRN		{return(TRN);}
UNTIL		{corc = true; return(UNTIL);}
WATCH		{return(WATCH);}
WHILE		{return(WHILE);}
WRITE		{return(WRITE);}

"*DATA"		{return(DATA);}

[A-Z][A-Z0-9]*	{yylval.node = intern_identifier(yytext); return(IDENTIFIER);}
-?{N}E-?{D}	{yylval.node = intern_number(yytext); return(NUMBER);}
-?{N}		{yylval.node = intern_number(yytext); return(NUMBER);}
'[^']*'		{yylval.node = intern_string(yytext); return(STRING);}

"**"		{return(POWER);}
"$"		{corc = true; return(POWER);}
[+*/().,=-]	{return(yytext[0]);}

[ \t]		;
\n		{
#ifdef FLEX_SCANNER
			yylineno++;
#endif /* FLEX_SCANNER */
		}

%%

static node *intern_identifier(char *str)
{
    register lvar *n;
    node	*new;

    new = (node *)NULL;
    for (n = idlist; n; n = n->next)
	if (strcmp(n->node->u.string, str) == 0)
	{
	    new = n->node;
	    break;
	}
    if (!n)
    {
	/* get a node */
	if ((new = (node *)malloc(sizeof(node)))==(node *)NULL)
	{
	    (void) puts("Out of memory while interning ID.");
	    exit(1);
	}

	/* stuff the node with the identifier vakue */
	new->type = IDENTIFIER;
	new->u.string = strdup(str);

	/* link it into the recognition list */
	n = (lvar *)calloc(sizeof(lvar), 1);
	n->next = idlist;
	n->node = new;
	idlist = n;
    }

#ifdef PARSEDEBUG
	if (verbose >= DEBUG_ALLOCATE)
	    (void) printf("intern: %p                        (IDENTIFIER: %s)\n",
		  new, str);
#endif /* PARSEDEBUG */

    return(new);
}

static node *intern_number(char *str)
{
    node	*new;
    scalar      numval = atos(str);	/* see cupl.h */

    /* get a node */
    if ((new = (node *)malloc(sizeof(node))) == (node *)NULL)
    {
	(void) puts(NOMEM);
	exit(1);
    }

    new->type = NUMBER;
    new->u.numval = numval;

#ifdef PARSEDEBUG
    if (verbose >= DEBUG_ALLOCATE)
	(void) printf("intern: %p                        (NUMBER: %f)\n", new, numval);
#endif /* PARSEDEBUG */
    return(new);
}

static node *intern_string(char *str)
{
    node	*new;

    /* get a node */
    if ((new = (node *)malloc(sizeof(node)))==(node *)NULL)
    {
	(void) puts("Out of memory while interning string.");
	exit(1);
    }

    str[strlen(str) - 1] = '\0';

    new->type = STRING;
    new->u.string = strdup(str + 1);

#ifdef PARSEDEBUG
    if (verbose >= DEBUG_ALLOCATE)
	(void) printf("intern: %p                        (STRING: '%s')\n",
		      new, new->u.string);
#endif /* PARSEDEBUG */

    return(new);
}

void yyerror(const char *str)
{
    (void) fprintf(stderr, "cupl: line %d, %s\n", yylineno, str);
}

int yywrap(void)
{
    return(EOF);
}

#ifdef MAIN
YYSTYPE yylval;
int	verbose;

main()
{
	int	t;

	while ((t = yylex()) > 0)
	{
		if (t == NUMBER)
			(void) printf("NUMBER:      %f\n", yylval.node->u.numval);
		else if (t == '\n')
			(void) printf("NEWLINE\n");
		else if (t == STRING)
			(void) printf("STRING:      %s\n", yylval.node->u.string);
		else if (t == IDENTIFIER)
			(void) printf("IDENTIFIER:  %s\n", yylval.node->u.string);
		else
			(void) printf("keyword %3d: %s\n", t, tokdump(t));
	}
}
#endif /* MAIN */

/* cupl.l ends here */
