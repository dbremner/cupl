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
	    if (data == (node *)NULL)
	    {
		warn("data list too short\n");
		v->elements[n] = 1;	/* 5-2 */
	    }
	    else
	    {
		if (data->car->type == NUMBER)
		    v->elements[n] = data->car->u.numval;
		else
		{
		    if (strcmp(tp->u.string , data->car->car->u.string))
			warn("data mismatch; expecting %s, saw %s\n",
			     tp->u.string , data->car->car->u.string);
		    v->elements[n] = data->car->cdr->u.numval;
		}

		data = data->cdr;
	    }
    }
}

static void cupl_write(node *tp)
/* evaluate a WRITE item */
{
    if (tp->type == STRING)
	(void) fputs(tp->u.string, stdout);
    else
	(void) printf("%f", tp->syminf->value.elements[0]);
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
	for_cdr(np, tree)
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
    node	*np, *next, *last;
    lvar	*lp;

    /* initially, all variables are scalars with zero values */
    for_symbols(lp)
    {
	lp->value.width = lp->value.depth = 1;
	lp->value.elements = (scalar *)malloc(sizeof(scalar));
	lp->value.elements[0] = 0;
    }

    /* locate the data pointer */
    data = last = (node *)NULL;
    for_cdr(np, tree)
    {
	if (np->car->type == DATA)
	{
	    data = np->car;
	    break;
	}
	last = np;
    }

    /* break the link to the data */
    if (!data)
	warn("no data supplied\n");
    else if (last)
	last->cdr = (node *)NULL;

    /* now execute the program */
    for (pc = tree; pc; pc = next)
    {
	next = pc->cdr;
	(void) cupl_eval(pc->car);
    }
}

/* execute.c ends here */


