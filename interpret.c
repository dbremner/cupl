#include <stdio.h>
#include <stdarg.h>
#include "cupl.h"
#include "tokens.h"

/* this structure represents a CUPL value */
typedef struct
{
    int		type;			/* type (syntax class) */
    int		rank;			/* 0, 1, or 2 */
    union
    {
	scalar	scalar;

	struct 
	{
	    int		width, depth;
	    scalar	*elements;
	}
	array;		/* a vector or matrix */
    } u;
}
value;

#ifdef PARSEDEBUG
/*
 * Note: This prettyprint routine tries to keep cons lists of elements of the
 * same type at the same indent level.  It does this by foregoing the normal
 * indent for the right-hand child of a cons if the child is the same type as
 * the parent cons.
 */
static void prettyprint(node *tree, int indent)
/* prettyprint a parse tree */
{
#define INDENT	2
    int	n;

    if (tree == (node *)NULL)
	return;

    for (n = 0; n < indent; n++)
	(void) putchar(' ');

    if (tree->type == NUMBER)
	(void) printf("NUMBER: %f\n", tree->u.numval);
    else if (tree->type == IDENTIFIER)
	(void) printf("IDENTIFIER: %s\n", tree->u.string);
    else if (tree->type == STRING)
	(void) printf("STRING: '%s'\n", tree->u.string);
    else
    {
	(void) printf("%-20s", tokdump(tree->type));
	if (verbose >= DEBUG_ALLOCATE)
	    (void) printf("                          (%x -> %x, %x)",
			  tree,
			  tree->u.n.left,
			  tree->u.n.right);
	(void) putchar('\n');
	prettyprint(tree->u.n.left,
		    indent + INDENT);
	prettyprint(tree->u.n.right,
		    indent + INDENT * (tree->type != tree->u.n.right->type));
    }
}
#endif /* PARSEDEBUG */

static void die(char *msg, ...)
/* complain of a fatal error and die */
{
    va_list	args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    exit(1);
}

static bool check_errors(node *tree)
/* look for inconsistencies in a program parse tree */
{
    node	*n;
    lvar	*lp;
    int		nlabels = 0;

    /* make backpointers */
    for (lp = idlist; lp; lp = lp->next)
	lp->node->syminf = lp;

    /* mark labels */
    for (n = tree; n; n = n->u.n.right)
	if (n->type != STATEMENT)
	    die("internal error: non-STATEMENT at top level");
	else if (n->u.n.left->type == LABEL)
	{
	    n->u.n.left->u.n.left->syminf->islabel = TRUE;
	    nlabels++;
	}

    /* report on the number of labels */
    if (verbose >= DEBUG_CHECKDUMP)
	if (nlabels == 0)
	    (void) printf("No labels.\n");
	else
	{
	    (void) printf("Labels:");
	    for (lp = idlist; lp; lp = lp->next)
		if (lp->islabel)
		    (void) printf(" %s", lp->node->u.string);
	    (void) printf("\n");
	}
}

void interpret(node *tree)
/* interpret a program parse tree */
{
#ifdef PARSEDEBUG
    if (verbose > DEBUG_PARSEDUMP)
	prettyprint(tree, 0);
#endif /* PARSEDEBUG */

    if (check_errors(tree))
	return;

    /* actual interpretation or compilation goes here */
}

/* interpret.c ends here */

