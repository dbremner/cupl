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

static void needspace(int w)
/* emit a LF if there are not more than w spaces left on the line */
{
    static int used;

    if (w == 0)
	used = 0;
    else if (w == -1)
    {
	if (used != linewidth)
	    (void) putchar('\n');
    }
    else if (used + w >= linewidth)
    {
	(void) putchar('\n');
	used = 0;
    }
    else
	used += w;
}

static void cupl_write(node *tp)
/* evaluate a WRITE item */
{
    if (tp == (node *)NULL)
    {
	needspace(fieldwidth);
	(void) printf("%-*s", fieldwidth, " ");
    }
    else if (tp->type == STRING)
    {
	needspace(fieldwidth);
	(void) printf("%-*s", fieldwidth, tp->u.string, stdout);
    }
    else
    {
	scalar	q;

	if (tp->type == FWRITE)
	{
	    needspace(fieldwidth);
	    q = tp->car->syminf->value.elements[0];
	}
	else
	{
	    q = tp->syminf->value.elements[0];

	    needspace(2 *fieldwidth);
	    (void) printf("%*s = ", fieldwidth -3, tp->u.string);
	}

	if (0.001 < abs(q) && abs(q) < 100000)
	    (void) printf("%*f", fieldwidth, q);
	else
	    (void) printf("%*E", fieldwidth, q);
    }
}

static void make_scalar(value *v, scalar i)
/* initialize a scalar value element */
{
    v->width = v->depth = 1;
    v->elements = (scalar *)malloc(sizeof(scalar));
    v->elements[0] = i;
}

static value cupl_eval(node *tree)
/* recursively evaluate a CUPL parse tree */
{
    value	leftside, rightside, result;
    node	*np;

    switch(tree->type)
    {
    case NUMBER:
	make_scalar(&result, tree->u.numval);
	return(result);

    case IDENTIFIER:
	return(tree->syminf->value);

    case READ:
	for_cdr(np, tree)
	    cupl_read(np->car);
	break;

    case WRITE:
	needspace(0);
	for_cdr(np, tree)
	    cupl_write(np->car);
	needspace(-1);
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
	make_scalar(&lp->value, 0);

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


