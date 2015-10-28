/*****************************************************************************

NAME
    cupl.y -- grammar for the CUPL language

SYNOPSIS
   int yyparse()		-- parse CUPL on stdin

DESCRIPTION
   This YACC grammar parses the CUPL language, as described in Appendix A of
R.J. Walker's manual.  All it does is build a parse tree for later code
generation and interpretation.

AUTHOR
   Eric S. Raymond <esr@snark.thyrsus.com>, November 1994.  The
author retains copyright on this implementation.  Permission for
nonprofit and educational use is granted to all; if you're planning to
make money with this or any code derived from it, check with the
author first.

LICENSE
 SPDX-License-Identifier: BSD-2-clause

****************************************************************************/
%{
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>

#ifdef PARSEDEBUG
static int statement_count;
#endif /* PARSEDEBUG */

#include "cupl.h"

#ifdef YYBISON
int yydebug;
#endif /* YYBISON */
%}

%start program

%union
{
    struct edon	*node;
}

/* keywords */
%token ABS ALL ALLOCATE AND ATAN BLOCK BY COMMENT COS DEC DET DOT ELSE END EXP
%token FLOOR FOR GE GO GT IDN IF INC INV LE LET LN LOG LT MAX MIN NE OR PERFORM
%token POSMAX POSMIN RAND READ SGM SIN SQRT STOP THEN TIMES TO TRC TRN
%token UNTIL WATCH WHILE WRITE DATA

%token <node> POWER
%token <node> STRING
%token <node> NUMBER
%token <node> IDENTIFIER
%token <node> TITLE

%token LPAREN	')'
%token RPAREN	'('
%token PLUS	'+'
%token MINUS	'-'
%token MULTIPLY	'*'
%token DIVIDE	'/'
%token EQUAL	'='
%token PERIOD	'.'
%token COMMA	','

%left <node> '+' '-'
%left <node> '*' '/'
%left <node> UMINUS
%nonassoc POWER

/* internal node types */
%token FWRITE
%token OG
%token UMINUS
%token SUBSCRIPT
%token STATEMENT
%token VARLIST
%token FORLIST
%token LABEL
%token IFELSE
%token DIMENSION
%token ISLICE JSLICE
%token ITERATE
%token FROM
%token TRIPLE

%type <node> prog command cond simple guard perform gosub iter expr rel alloc
%type <node> subscr datal ditem triple expl readl writel witem allocl varlist
%type <node> minl maxl

%%	/* beginning of rules section */

/* a program description consists of a sequence of statements */
program :    prog			{interpret($1);}
	;

prog	:    command prog
		{
		    $$ = cons(STATEMENT, $1, $2);
#ifdef PARSEDEBUG
		    $$->number = ++statement_count;
#endif /* PARSEDEBUG */
		}
	|    IDENTIFIER command prog
		{
		    $$ = cons(STATEMENT, cons(LABEL, $1, $2), $3);
#ifdef PARSEDEBUG
		    $$->number = ++statement_count;
#endif /* PARSEDEBUG */
		}
	|    /* EMPTY */
		{$$ = (node *)NULL;}
	;

/* statement syntax */

command	:    simple			{$$ = $1;}
	|    cond			{$$ = $1;}
	|    perform			{$$ = $1;}
	|    BLOCK			{$$ = cons(BLOCK, NULLNODE, NULLNODE);}
	|    END			{$$ = cons(END, NULLNODE, NULLNODE);}
	|    DATA datal			{$$ = $2;}
	|    TITLE			{$$ = cons(WRITE, $1, NULLNODE);;}
	;

cond	:    IF guard THEN simple
		{$$ = cons(IF, $2, $4);}
	|    IF guard ELSE simple
		{$$ = cons(IFELSE, $2, cons(THEN, $4, (node *)NULL));}
	|    IF guard THEN simple ELSE simple
		{$$ = cons(IFELSE, $2, cons(THEN, $4, $6));}
	;

ditem	:    NUMBER			{$$ = $1;}
	|    IDENTIFIER '=' NUMBER	{$$ = cons(LET,$1, $3);}
	;

datal	:    ditem ',' datal		{$$ = cons(DATA, $1, $3);}
	|    ditem     datal		{$$ = cons(DATA, $1, $2);}
	|    ditem			{$$ = cons(DATA, $1, NULLNODE);}
	;

gosub	:   PERFORM IDENTIFIER		{$$ = cons(PERFORM, $2, NULLNODE);}
	;

perform	:    gosub			{$$ = $1;}
	|    gosub expr TIMES		{$$ = cons(TIMES, $2, $1);}
	|    gosub WHILE guard		{$$ = cons(WHILE, $3, $1);}
	|    gosub UNTIL guard		{$$ = cons(UNTIL, $3, $1);}

	|    gosub FOR IDENTIFIER '=' expl
		{$$ = cons(FOR,   cons('=', $3, $5),                     $1);}
	|    gosub FOR IDENTIFIER '=' expr iter
		{$$ = cons(FOR,   cons(ITERATE, $3, cons(FROM, $5, $6)), $1);}
	;

iter	:    TO expr BY expr		{$$ = cons(TO, $2, $4);}
	|    BY expr TO expr		{$$ = cons(TO, $4, $2);}
	|    TO expr			{$$ = cons(TO, $2, (node *)NULL);}
	;

triple  :    '(' expr ',' expr ',' expr ')'
		{$$ = cons(TRIPLE, $2, cons(ITERATE, $4, $6));}
	;

expl	:    expr ',' expl		{$$ = cons(FORLIST, $1, $3);}
	|    triple ','	expl		{$$ = cons(FORLIST, $1, $3);}
	|    expr			{$$ = cons(FORLIST, $1, NULLNODE);}
	|    triple			{$$ = cons(FORLIST, $1, NULLNODE);}
	;

/* guard syntax */

/*
 * this relaxes the pure-AND/pure-OR rule on guards, in order to avoid
 * a parser ambiguity; we can check the rule at runtime
 */
guard	:    rel			{$$ = $1;}
	|    rel AND rel		{$$ = cons(AND, $1, $3);}
	|    rel OR rel			{$$ = cons(OR,  $1, $3);}
	;

rel	:    expr '=' expr		{$$ = cons('=', $1, $3);}
	|    expr NE expr		{$$ = cons(NE,  $1, $3);}
	|    expr LE expr		{$$ = cons(LE,  $1, $3);}
	|    expr GE expr		{$$ = cons(GE,  $1, $3);}
	|    expr LT expr		{$$ = cons(LT,  $1, $3);}
	|    expr GT expr		{$$ = cons(GT,  $1, $3);}
	;

/* simple statement types */

simple	:    LET IDENTIFIER '=' expr	{$$ = cons(LET, $2, $4);}
	|    INC IDENTIFIER BY expr	{$$ = cons(LET, $2, cons(PLUS,  $2, $4));}    /* CORC */
	|    DEC IDENTIFIER BY expr	{$$ = cons(LET, $2, cons(MINUS, $2, $4));}    /* CORC */
	|    GO TO IDENTIFIER		{$$ = cons(GO, $3, NULLNODE);}
	|    GO TO IDENTIFIER END	{$$ = cons(OG, $3, NULLNODE);}
	|    READ readl			{$$ = $2;}
	|    WRITE ALL			{$$ = cons(WRITE, cons(ALL, NULLNODE, NULLNODE), NULLNODE);}
	|    WRITE writel		{$$ = $2;}
	|    ALLOCATE allocl		{$$ = $2;}
	|    WATCH varlist		{$$ = $2;}
	|    STOP			{$$ = cons(STOP, NULLNODE, NULLNODE);}
	;

readl	:    IDENTIFIER ',' readl	{$$ = cons(READ, $1, $3);}
	|    IDENTIFIER			{$$ = cons(READ, $1, NULLNODE);}
	;

writel	:    witem ',' writel		{$$ = cons(WRITE, $1, $3);}
	|    witem			{$$ = cons(WRITE, $1, NULLNODE);}
	;

witem	:    IDENTIFIER			{$$ = $1;}
	|    '/' IDENTIFIER		{$$ = cons(FWRITE, $2, NULLNODE);}
	|    STRING			{$$ = $1;}
	|    /* EMPTY */		{$$ = (node *)NULL;}
	;

allocl	:    alloc ',' allocl		{$$ = cons(VARLIST, $1, $3);}
	|    alloc			{$$ = cons(VARLIST, $1, NULLNODE);}
	;

alloc   :    IDENTIFIER '(' expr ')'
		{$$ = cons(ALLOCATE, $1, $3);}
	|    IDENTIFIER '(' expr  ',' expr ')'
		{$$ = cons(ALLOCATE, $1, cons(DIMENSION, $3, $5));}
	;

varlist	:    IDENTIFIER ',' varlist	{$$ = cons(WATCH, $1, $3);}
	|    IDENTIFIER			{$$ = cons(WATCH, $1, NULLNODE);}
	;

/* expression syntax */

expr	:    NUMBER			{$$ = $1;}
	|    IDENTIFIER			{$$ = $1;}

	|    '(' expr ')'		{$$ = $2;}
	|    expr '+' expr		{$$ = cons(PLUS, $1, $3);}
	|    expr '-' expr		{$$ = cons(MINUS, $1, $3);}
	|    '-' expr %prec UMINUS	{$$ = cons(UMINUS, NULLNODE, $2);}
	|    expr '*' expr		{$$ = cons(MULTIPLY, $1, $3);}
	|    expr '/' expr		{$$ = cons(DIVIDE, $1, $3);}
	|    expr POWER expr		{$$ = cons(POWER, $1, $3);}

	|    IDN			{$$ = cons(IDN, NULLNODE, NULLNODE);}
	|    subscr			{$$ = $1;}

	|    ABS '(' expr ')'		{$$ = cons(ABS, NULLNODE, $3);} 
	|    ATAN '(' expr ')'		{$$ = cons(ATAN, NULLNODE, $3);} 
	|    COS '(' expr ')'		{$$ = cons(COS, NULLNODE, $3);} 
	|    EXP '(' expr ')'		{$$ = cons(EXP, NULLNODE, $3);} 
	|    FLOOR '(' expr ')'		{$$ = cons(FLOOR, NULLNODE, $3);} 
	|    LOG '(' expr ')'		{$$ = cons(LOG, NULLNODE, $3);} 
	|    SQRT '(' expr ')'		{$$ = cons(SQRT, NULLNODE, $3);} 
	|    SIN '(' expr ')'		{$$ = cons(SIN, NULLNODE, $3);} 
	|    RAND '(' expr ')'		{$$ = cons(RAND, NULLNODE, $3);} 
	|    DET '(' expr ')'		{$$ = cons(DET, NULLNODE, $3);} 
	|    DOT '(' expr ',' expr ')'	{$$ = cons(DOT, $3, $5);} 
	|    INV '(' expr ')'		{$$ = cons(INV, NULLNODE, $3);} 
	|    POSMAX '(' expr ')'	{$$ = cons(POSMAX, NULLNODE, $3);} 
	|    POSMIN '(' expr ')'	{$$ = cons(POSMIN, NULLNODE, $3);} 
	|    SGM '(' expr ')'		{$$ = cons(SGM, NULLNODE, $3);} 
	|    TRC '(' expr ')'		{$$ = cons(TRC, NULLNODE, $3);} 
	|    TRN '(' expr ')'		{$$ = cons(TRN, NULLNODE, $3);} 

	|    MAX '(' expr ',' maxl ')'	{$$ = cons(MAX, $3, $5);}
	|    MIN '(' expr ',' minl ')'	{$$ = cons(MIN, $3, $5);}
	;

maxl	:   expr ',' maxl		{$$ = cons(MAX, $1, $3);}
	|   expr			{$$ = cons(MAX, $1, NULLNODE);}
	;

minl	:   expr ',' minl		{$$ = cons(MIN, $1, $3);}
	|   expr			{$$ = cons(MIN, $1, NULLNODE);}
	;

subscr	:    IDENTIFIER '(' expr ')'
		{$$ = cons(SUBSCRIPT, $1, $3);}
	|    IDENTIFIER '(' expr  ',' expr ')'
		{$$ = cons(SUBSCRIPT, $1, cons(DIMENSION, $3, $5));}
	|    IDENTIFIER '(' expr  ',' '*' ')'
		{$$ = cons(JSLICE, $1, $3);}
	|    IDENTIFIER '(' '*'  ',' expr ')'
		{$$ = cons(ISLICE, $1, $5);}
	;
%%

node *cons(int op, node *left, node *right)
/* make a cons for a binary operation */
/*int	op;		opcode */
/*node	*left, *right;	child nodes */
{
    node	*new;

    /* get a node */
    if ((new = (node *)malloc(sizeof(node))) == (node *)NULL)
    {
	(void) puts(NOMEM);
	exit(1);
    }

    new->type = op;
    new->u.n.left = left;
    new->u.n.right = right;

#ifdef PARSEDEBUG
    if (verbose >= DEBUG_ALLOCATE)
	(void) printf("cons:   %p -> %p %p (%s)\n",
		      new, left, right, tokdump(op));
#endif /* PARSEDEBUG */

    return(new);
}

/* cupl.y ends here */
