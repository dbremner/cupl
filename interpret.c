/*****************************************************************************

NAME
   interpret.c -- parse-tree interpretation

SYNOPSIS
   void interpret(node *tree)	-- prepare and execute a parse tree

DESCRIPTION
   This code does interpretation, static checking, and label resolution
of a CUPL parse tree.  Actual execution is handed off to execute(). 

*****************************************************************************/
/*LINTLIBRARY*/
#include <stdio.h>
#include <stdarg.h>
#include "cupl.h"
#include "tokens.h"

#define car	u.n.left
#define cdr	u.n.right

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
    else if (tree->type == STATEMENT && indent > 0)
	(void) printf("-> STATEMENT %d\n", tree->number);
    else
    {
	if (tree->type == STATEMENT)
	    (void) printf("STATEMENT %2d       ", tree->number);
	else
	    (void) printf("%-20s", tokdump(tree->type));

	if (verbose >= DEBUG_ALLOCATE)
	    (void) printf("                          (%x -> %x, %x)",
			  tree,
			  tree->car,
			  tree->cdr);
	(void) putchar('\n');
	prettyprint(tree->car,
		    indent + INDENT);
	prettyprint(tree->cdr,
		    indent + INDENT * (tree->type != tree->cdr->type));
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

static bool recursive_apply(node *tree, bool (*fun)(node *))
/* apply fun recursively to tree, short-circuiting on FALSE return */
{
    if (tree == (node *)NULL)
	return(TRUE);

    if (ATOMIC(tree->type))
	return(fun(tree));
    else if (recursive_apply(tree->car, fun) && recursive_apply(tree->cdr, fun))
	return(fun(tree));
    else
	return(FALSE);

}

static bool r_mark_labels(node *tp)
/* record label references */
{
    /* count label definitions */
    if (tp->type == LABEL)
	tp->car->syminf->labeldef++;

    /* count label references */
    if (LABELREF(tp->type))
	if (tp->car && tp->car->type == IDENTIFIER)
	    tp->car->syminf->labelref++;
	else if (tp->cdr && tp->cdr->type == IDENTIFIER)
	    tp->cdr->syminf->labelref++;

    /* count identifier assignments */
    if (VARSET(tp->type))
    {
	if (tp->car->type == IDENTIFIER)
	{
#ifdef ODEBUG
	    (void) printf("left  operand %8s of %8s assigned\n",
			  tp->car->u.string, tokdump(tp->type));
#endif /* ODEBUG */
	    tp->car->syminf->assigned++;
	}
    }
    else if (!ATOMIC(tp->type))
    {
	if (tp->car && tp->car->type == IDENTIFIER && LEFTREF(tp->type))
	{
#ifdef ODEBUG
	    (void) printf("left  operand %8s of %8s used\n",
			  tp->car->u.string, tokdump(tp->type));
#endif /* ODEBUG */
	    tp->car->syminf->used++;
	}
	

	if (tp->cdr && tp->cdr->type == IDENTIFIER && RIGHTREF(tp->type))
	{
#ifdef ODEBUG
	    (void) printf("right operand %8s of %8s used\n",
			  tp->cdr->u.string, tokdump(tp->type));
#endif /* ODEBUG */
	    tp->cdr->syminf->used++;
	}
    }

    return(TRUE);
}

static bool check_errors(node *tree)
/* look for inconsistencies in a program parse tree */
{
    node	*n;
    lvar	*lp;

    /* simple sanity check */
    for_cdr(n, tree)
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

    return(FALSE);
}

static bool r_label_rewrite(node *tp)
/* resolve label references */
{
    /*
     * KISS -- this code may do extra work, but it needs no special
     * knowledge about node types.
     */
    if (LABELREF(tp->type))
    {
	node *left = tp->car;
	node *right = tp->cdr;

	if (left && left->type == IDENTIFIER && left->syminf->labelref)
	{
#ifdef ODEBUG
	    (void) printf("patching label left reference to %s\n", left->u.string);
#endif /* ODEBUG */
	    tp->car = left->syminf->target;
	}
	if (right && right->type == IDENTIFIER && right->syminf->labelref)
	{
#ifdef ODEBUG
	    (void) printf("patching label right reference to %s\n", right->u.string);
#endif /* ODEBUG */
	    tp->cdr = right->syminf->target;
	}
    }

    return(TRUE);
}

static void rewrite(node *tree)
/* resolve labels */
{
    node	*np;

    /* first, make a pointer from each label to its destination statement */
    for_cdr(np, tree)
	if (np->car->type == LABEL)
	    np->car->car->syminf->target = np;

#ifdef _FOO_
    /* pull blocks out of the main line */
    for_cdr(np, tree)
    {
	node *sp = np->car;

	if (sp && sp->type == LABEL && sp->cdr->type == BLOCK)
	{
	    node	*endnode;
	    bool	found = FALSE;

printf("statement %d, label '%s'\n", np->number, sp->car->u.string);

	    for (endnode = np->cdr; endnode; endnode = endnode->cdr)
		if (endnode->car->type == LABEL
			&& endnode->car->cdr->type == BLOCK
			&& strcmp(sp->car->u.string,
				  endnode->car->car->u.string) == 0)
		{
		    found = TRUE;
		    break;
		}
	    if (!found)
		warn("no END matching block label %s\n", sp->car->u.string);
	    else
		(void) printf("found END for %s\n", sp->car->u.string);
	}
    }
#endif /* _FOO_ */

    /* now, hack label references to eliminate name references */
    recursive_apply(tree, r_label_rewrite);

}

void interpret(node *tree)
/* interpret a program parse tree */
{
#ifdef PARSEDEBUG
    /* statement conses are made in reverse order; deal with this */
    {
	node	*np;
	int statement_count = 0;

	for_cdr(np, tree)
	{
	    if (np->type == STATEMENT)
		++statement_count;
	}

	for_cdr(np, tree)
	    if (np->type == STATEMENT)
		np->number = statement_count - np->number + 1;
    }
#endif /* PARSEDEBUG */

    if (check_errors(tree))
	return;
    rewrite(tree);

#ifdef PARSEDEBUG
    if (verbose >= DEBUG_PARSEDUMP)
	prettyprint(tree, 0);
#endif /* PARSEDEBUG */

    execute(tree);
}

/* interpret.c ends here */
