/*****************************************************************************

NAME
   execute.c -- parse-tree execution

SYNOPSIS
   void execute(node *tree)	-- execute a parse tree

DESCRIPTION 
   This code does execution of a CUPL parse tree.  Actual execution is
handed off to execute().

*****************************************************************************/
/*LINTLIBRARY*/
#include "cupl.h"
#include "tokens.h"

#define car	u.n.left
#define cdr	u.n.right

static node *pc;	/* the statement node to evaluate next */

static void cupl_read(node *tp)
/* evaluate a READ item */
{
}

static void cupl_write(node *tp)
/* evaluate a WRITE item */
{
}

static value cupl_eval(node *tree)
/* recursively evaluate a CUPL parse tree */
{
    value	leftside, rightside;
    node	*np;

    switch(tree->type)
    {
    case READ:
	for_cdr(np, tree->car)
	    cupl_read(np->car);
	break;

    case WRITE:
	for_cdr(np, tree->car)
	    cupl_write(np->car);
	break;

    default:
	die("unknown node type %d (%s), cannot execute\n",tokdump(tree->type));
	break;
    }
}

void execute(node *tree)
/* execute a CUPL program described by a parse tree */
{
#ifdef FOO
    node	*next;

    for (pc = tree; pc; pc = next)
    {
	next = pc->cdr;
	(void) cupl_eval(pc->car);
    }
#endif /* FOO */
}

/* execute.c ends here */
