#include <stdio.h>
#include <stdarg.h>
#include "cupl.h"
#include "tokens.h"

/*
 * Define token classes for consistency checks here.  The theory is
 * that by separating this from the tree-traversal machinery below, we
 * may be able to parametrize for different source languages someday.
 */
/* does the node represent an atom? */
#define ATOMIC(n)	((n) == IDENTIFIER || (n) == NUMBER || (n) == STRING)
/* does the node reference a label? */
#define LABELREF(n)	((n) == GO || (n) == OG || (n) == PERFORM \
			 || (n) == TIMES || (n) == WHILE || (n) == FOR)
/* does the node set a variable? */
#define VARSET(n)	((n) == LET || (n) == READ || (n) == ITERATE)
/* does the (non-VARSET) node refer to its left operand? */ 
#define LEFTREF(n)	((n) != FOR && (n) != GO && (n) != OG && (n) != LABEL \
				&& (n) != ALLOCATE && (n) != WATCH \
				&& (n) != ITERATE && (n) != FROM \
				&& (n) != LABEL && (n) != TIMES \
				&& (n) != WHILE)
/* does the (non-VARSET) node refer to its right operand? */
#define RIGHTREF(n)	((n) != PERFORM)

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

static void warn(char *msg, ...)
/* warn of an error and die */
{
    va_list	args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    exit(1);
}

static void die(char *msg, ...)
/* complain of a fatal error and die */
{
    va_list	args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    exit(1);
}

static bool r_mark_labels(node *tp)
/* record label references */
{
    /* count label definitions */
    if (tp->type == LABEL)
	tp->u.n.left->syminf->labeldef++;

    /* count label references */
    if (LABELREF(tp->type))
	if (tp->u.n.left && tp->u.n.left->type == IDENTIFIER)
	    tp->u.n.left->syminf->labelref++;
	else if (tp->u.n.right && tp->u.n.right->type == IDENTIFIER)
	    tp->u.n.right->syminf->labelref++;

    /* count identifier assignments */
    if (VARSET(tp->type))
    {
	if (tp->u.n.left->type == IDENTIFIER)
	{
#ifdef ODEBUG
	    (void) printf("left  operand %8s of %8s assigned\n",
			  tp->u.n.left->u.string, tokdump(tp->type));
#endif /* ODEBUG */
	    tp->u.n.left->syminf->assigned++;
	}
    }
    else if (!ATOMIC(tp->type))
    {
	if (tp->u.n.left && tp->u.n.left->type == IDENTIFIER && LEFTREF(tp->type))
	{
#ifdef ODEBUG
	    (void) printf("left  operand %8s of %8s used\n",
			  tp->u.n.left->u.string, tokdump(tp->type));
#endif /* ODEBUG */
	    tp->u.n.left->syminf->used++;
	}
	

	if (tp->u.n.right && tp->u.n.right->type == IDENTIFIER && RIGHTREF(tp->type))
	{
#ifdef ODEBUG
	    (void) printf("right operand %8s of %8s used\n",
			  tp->u.n.right->u.string, tokdump(tp->type));
#endif /* ODEBUG */
	    tp->u.n.right->syminf->used++;
	}
    }

    return(TRUE);
}

bool recursive_apply(node *tree, bool (*fun)(node *))
/* apply fun recursively to tree, short-circuiting on FALSE return */
{
    if (tree == (node *)NULL)
	return(TRUE);

    if (ATOMIC(tree->type))
	return(fun(tree));
    else if (recursive_apply(tree->u.n.left, fun) && recursive_apply(tree->u.n.right, fun))
	return(fun(tree));
    else
	return(FALSE);

}

static bool check_errors(node *tree)
/* look for inconsistencies in a program parse tree */
{
    node	*n;
    lvar	*lp;

#define for_symbols(s)    for (s = idlist; s; s = s->next)

    /* simple sanity check */
    for (n = tree; n; n = n->u.n.right)
	if (n->type != STATEMENT)
	    die("internal error: non-STATEMENT at top level");

    /* make backpointers */
    for_symbols(lp)
	lp->node->syminf = lp;

    /* mark labels */
    recursive_apply(tree, r_mark_labels);

    /* check for label/variable consistency */
    for_symbols(lp)
	if (lp->labelref && !lp->labeldef)
	    die("label %s used but not defined\n", lp->node->u.string);
	else if ((lp->labelref || lp->labeldef) && (lp->assigned || lp->used))
	    die("%s used as both variable and label\n");
	else if (!lp->labelref && lp->labeldef)
	    warn("label %s defined but never used\n", lp->node->u.string);
        else if (lp->used && !lp->assigned)
	    warn("variable %s used but not set\n", lp->node->u.string);
        else if (!lp->used && lp->assigned)
	    warn("variable %s set but not used\n", lp->node->u.string);

    /* describe all labels and variables */
    if (verbose >= DEBUG_CHECKDUMP)
    {
	int		nlabels = 0;

	for_symbols(lp)
	    if (lp->labeldef)
		nlabels++;

	if (nlabels == 0)
	    (void) printf("No labels.\n");
	else
	{
	    (void) printf("Labels:\n");
	    for_symbols(lp)
		if (lp->labeldef)
		    (void) printf("    %8s: %d reference(s)\n",
				  lp->node->u.string, 
				  lp->labelref);
	}

	/* static counts for variables */
	(void) printf("Variables:\n");
	for_symbols(lp)
	    if (!lp->labeldef)
		(void) printf("    %8s: %d assignments, %d reference(s)\n",
			      lp->node->u.string, lp->assigned, lp->used);
    }
}

void interpret(node *tree)
/* interpret a program parse tree */
{
#ifdef PARSEDEBUG
    if (verbose >= DEBUG_PARSEDUMP)
	prettyprint(tree, 0);
#endif /* PARSEDEBUG */

    if (check_errors(tree))
	return;

    /* actual interpretation or compilation goes here */
}

/* interpret.c ends here */

