%{
/* cupl.l -- lexical analyzer for the CUPL language */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include "tokens.h"
#include "cupl.h"

extern char *strdup(const char *s);

typedef struct lvar_t
{
    struct lvar_t	*next;
    node		*node;
}
lvar;

static lvar *idlist;

static node *intern_number(double);
static node *intern_identifier(char *);
static node *intern_string(char *);
%}

D	[0-9][0-9]*
N	[0-9.]+

%%
COMMENT.*\n	;

ABS		{return(ABS);}
ALL		{return(ALL);}
ALLOCATE	{return(ALLOCATE);}
AND		{return(AND);}
ATAN		{return(ATAN);}
BLOCK		{return(BLOCK);}
BY		{return(BY);}
COMMENT		{return(COMMENT);}
COS		{return(COS);}
DET		{return(DET);}
DOT		{return(DOT);}
ELSE		{return(ELSE);}
END		{return(END);}
EXP		{return(EXP);}
FLOOR		{return(FLOOR);}
FOR		{return(FOR);}
GE		{return(GE);}
GO		{return(GO);}
GT		{return(GT);}
IDN		{return(IDN);}
IF		{return(IF);}
INV		{return(INV);}
LE		{return(LE);}
LET		{return(LET);}
LN		{return(LN);}
LOG		{return(LOG);}
LT		{return(LT);}
MAX		{return(MAX);}
MIN		{return(MIN);}
NE		{return(NE);}
OR		{return(OR);}
PERFORM		{return(PERFORM);}
POSMAX		{return(POSMAX);}
POSMIN		{return(POSMIN);}
RAND		{return(RAND);}
READ		{return(READ);}
SGM		{return(SGM);}
SIN		{return(SIN);}
SQRT		{return(SQRT);}
STOP		{return(STOP);}
THEN		{return(THEN);}
TIMES		{return(TIMES);}
TO		{return(TO);}
TRC		{return(TRC);}
TRN		{return(TRN);}
WATCH		{return(WATCH);}
WHILE		{return(WHILE);}
WRITE		{return(WRITE);}

"*DATA"		{return(DATA);}

[A-Z][A-Z0-9]*	{yylval.node = intern_identifier(yytext); return(IDENTIFIER);}
-?{N}E-?{D}	{yylval.node = intern_number(atof(yytext)); return(NUMBER);}
-?{N}		{yylval.node = intern_number(atof(yytext)); return(NUMBER);}
'[^']*'		{yylval.node = intern_string(yytext); return(STRING);}

"**"		{return(POWER);}
[+*/().,=-]	{return(yytext[0]);}

[ \t\n]		;

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
	n = (lvar *)malloc(sizeof(lvar));
	n->next = idlist;
	n->node = new;
	idlist = n;

#ifdef PARSEDEBUG
	if (verbose >= 2)
	    (void) printf("intern: %9x                        (IDENTIFIER: %s)\n",
			  new, str);
#endif /* PARSEDEBUG */
    }

    return(new);
}

static node *intern_number(double numval)
{
    node	*new;

    /* get a node */
    if ((new = (node *)malloc(sizeof(node))) == (node *)NULL)
    {
	(void) puts(NOMEM);
	exit(1);
    }

    new->type = NUMBER;
    new->u.numval = numval;

#ifdef PARSEDEBUG
    if (verbose >= 2)
	(void) printf("intern: %x (NUMBER: %d)\n", new, numval);
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
    if (verbose >= 2)
	(void) printf("intern: %9x                        (STRING: '%s')\n",
		      new, new->u.string);
#endif /* PARSEDEBUG */

    return(new);
}

void yyerror(const char *str)
{
    (void) fprintf(stderr, "cupl: line %d, %s\n", yylineno, str);
}

int yywrap()
{
    return(EOF);
}

#ifdef MAIN
YYSTYPE yylval;

main()
{
	int	t;

	while ((t = yylex()) > 0)
	{
		if (t == NUMBER)
			(void) printf("NUMBER:      %f\n", yylval.numval);
		else if (t == NEWLINE)
			(void) printf("NEWLINE\n");
		else if (t == STRING)
			(void) printf("STRING:      %s\n", yylval.string);
		else if (t == IDENTIFIER)
			(void) printf("IDENTIFIER:  %s\n", yylval.string);
		else
			(void) printf("keyword %3d: %s\n", t, tokdump(t));
	}
}
#endif /* MAIN */

/* cupl.l ends here */
