#include <stdio.h>
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

/* this structure represents a CUPL variable */
typedef struct rav
{
    char	name[MAXNAME + 1];	/* name of variable */
    value	value;
}
variable;

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
	if (verbose >= 2)
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

void interpret(node *tree)
/* interpret a program parse tree */
{
#ifdef PARSEDEBUG
    if (verbose > 0)
	prettyprint(tree, 0);
#endif /* PARSEDEBUG */
}

/* interpret.c ends here */

