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
#include <stdio.h>
#include "cupl.h"
#include "tokens.h"
#include "nodetype.h"

#define car	u.n.left
#define cdr	u.n.right

static node *pc;	/* the statement node to evaluate next */
static node *data;	/* pointer to *DATA item to be grabbed next */

static void cupl_read(node *tp)
/* evaluate a READ item */
{
    if (tp->type != IDENTIFIER)
	die("internal error -- garbled READ list\n");
    else
    {
	value *v = &(tp->syminf->value);
	int	n;

	for (n = 0; n < v->width * v->depth; n++)
	    if (ATOMIC(data->car->type))
	    {
		v->elements[n] = data->car->u.numval;
		data = data->cdr;
	    }
	/* code to handle conses expressing check variables goes here */
    }
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
	for_cdr(np, tree)
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
    node	*np, *next;
    lvar	*lp;

    /* initially, all variables are scalars with zero values */
    for_symbols(lp)
    {
	lp->value.width = lp->value.depth = 1;
	lp->value.elements = (scalar *)malloc(sizeof(scalar));
	lp->value.elements[0] = 0;
    }

    /* locate the data pointer */
    data = (node *)NULL;
    for_cdr(np, tree)
	if (np->car->type == DATA)
	{
	    data = np->car;
	    break;
	}
    if (!data)
	warn("no data supplied\n");

    for (pc = tree; pc; pc = next)
    {
	next = pc->cdr;
	(void) cupl_eval(pc->car);
    }
}

/* execute.c ends here */


