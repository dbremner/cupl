/*****************************************************************************

NAME
   interpret.c -- parse-tree interpretation

SYNOPSIS
   void interpret(node *tree)	-- prepare and execute a parse tree

DESCRIPTION
   This code does interpretation, static checking, and label resolution
of a CUPL parse tree.  Actual execution is handed off to execute().

NOTE
   The NOTE: comments describe a few things that will need to be done 
differently if we get a compiler back end.

LICENSE
  SPDX-License-Identifier: BSD-2-clause

*****************************************************************************/
/*LINTLIBRARY*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cupl.h"
#include "tokens.h"

/* nodetype.h -- macros that describe the semantics of nodes */

/*
 * Define token classes for consistency checks here.  The theory is
 * that by separating this from the tree-traversal machinery below, we
 * may be able to parametrize for different source languages someday.
 */
/* does the node represent an atom? */
#define ATOMIC(n)	((n) == IDENTIFIER || (n) == NUMBER || (n) == STRING)
/* does the node reference a statement label? */
#define SLABELREF(n)	((n) == GO)
/* does the node reference a block label? */
#define BLABELREF(n)	((n) == PERFORM || (n) == END || (n) == OG)
/* does the node set a variable? */
#define VARSET(n)	((n) == LET || (n) == READ || (n) == ITERATE || (n) == '=')
/* does the (non-VARSET) node refer to its left operand as a variable? */ 
#define LEFTREF(n)	((n) != FOR && (n) != GO && (n) != OG && (n) != LABEL \
				&& (n) != ALLOCATE && (n) != WATCH \
				&& (n) != ITERATE && (n) != FROM \
		     		&& (n) != PERFORM)
/* does the (non-VARSET) node refer to its right operand? */
#define RIGHTREF(n)	((n) != PERFORM)

/* nodetype.h ends here */

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
    {
	if (tree->car->type == LABEL)
	    (void) printf("-> %s (STATEMENT %d)\n", 
			  tree->car->car->u.string, tree->number);
	else
	    (void) printf("-> STATEMENT %d\n", tree->number);
    }
    else
    {
	if (tree->type == STATEMENT)
	    (void) printf("STATEMENT %2d       ", tree->number);
	else
	    (void) printf("%-20s", tokdump(tree->type));

	if (verbose >= DEBUG_EXECUTE)
	    (void) printf("                          (%p -> %p, %p)",
			  tree,
			  tree->car,
			  tree->cdr);
	(void) putchar('\n');
	prettyprint(tree->car,
		    indent + INDENT);
	prettyprint(tree->cdr,
		    indent + INDENT * (tree->cdr && (tree->type != tree->cdr->type)));
    }
}
#endif /* PARSEDEBUG */

static bool recursive_apply(node *tree, bool (*fun)(node *))
/* apply fun recursively to tree, short-circuiting on false return */
{
    if (tree == (node *)NULL)
	return(true);

    if (ATOMIC(tree->type))
	return(fun(tree));
    else if (recursive_apply(tree->car, fun) && recursive_apply(tree->cdr, fun))
	return(fun(tree));
    else
	return(false);

}

static bool r_mark_labels(node *tp)
/* record label references */
{
    if (ATOMIC(tp->type))
	return(true);

    /* count label definitions */
    if (tp->type == LABEL)
	if (tp->cdr->type == BLOCK)
	    tp->car->syminf->blabeldef++;
	else if (tp->cdr->type != END)
	    tp->car->syminf->slabeldef++;

    /* count block label references */
    if (BLABELREF(tp->type))
	if (tp->car && tp->car->type == IDENTIFIER)
	    tp->car->syminf->blabelref++;
	else if (tp->cdr && tp->cdr->type == IDENTIFIER)
	    tp->cdr->syminf->blabelref++;

    /* count statement label references */
    if (SLABELREF(tp->type))
	if (tp->car && tp->car->type == IDENTIFIER)
	    tp->car->syminf->slabelref++;
	else if (tp->cdr && tp->cdr->type == IDENTIFIER)
	    tp->cdr->syminf->slabelref++;

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

    /* WRITE ALLs reference everything */
    if (tp->type == WRITE && tp->car && tp->car->type == ALL)
    {
	lvar	*lp;

	for_symbols(lp)
	    if (lp->assigned)
		lp->used++;
    }

    return(true);
}

static bool mung_corc_labels(node *tp)
{
    if (tp->type == GO && tp->car->syminf->blabeldef > 0)
    {
	tp->type = OG;
	tp->car->syminf->slabelref--;
	tp->car->syminf->blabelref++;
    }

    return(true);
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

    /* map CORC's GO TO <block> to CUPL's GO TO <block> END */
    if (corc)
	recursive_apply(tree, mung_corc_labels);

    /* describe all labels and variables */
    if (verbose >= DEBUG_CHECKDUMP)
    {
	int		nlabels;

	nlabels = 0;
	for_symbols(lp)
	    if (lp->blabeldef)
		nlabels++;

	if (nlabels == 0)
	    (void) printf("No block labels.\n");
	else
	{
	    (void) printf("Block labels:\n");
	    for_symbols(lp)
		if (lp->blabeldef)
		    (void) printf("    %8s: %d reference(s)\n",
				  lp->node->u.string, 
				  lp->blabelref);
	}

	nlabels = 0;
	for_symbols(lp)
	    if (lp->blabeldef)
		nlabels++;

	if (nlabels == 0)
	    (void) printf("No statement labels.\n");
	else
	{
	    (void) printf("Statement labels:\n");
	    for_symbols(lp)
		if (lp->slabeldef)
		    (void) printf("    %8s: %d reference(s)\n",
				  lp->node->u.string, 
				  lp->slabelref);
	}

	/* static counts for variables */
	(void) printf("Variables:\n");
	for_symbols(lp)
	    if (lp->assigned || lp->used)
		(void) printf("    %8s: %d assignments, %d reference(s)\n",
			      lp->node->u.string, lp->assigned, lp->used);
    }

    /* check for label/variable consistency */
    for_symbols(lp)
	if ((lp->blabelref || lp->blabeldef) + (lp->slabelref || lp->slabeldef) + (lp->assigned || lp->used) > 1)
	    die("%s has conflicting uses\n", lp->node->u.string);
	else if (lp->blabelref && !lp->blabeldef)
	    die("block label %s used but not defined\n", lp->node->u.string);
	else if (!lp->blabelref && lp->blabeldef)
	    warn("block label %s defined but never used\n",lp->node->u.string);
	else if (lp->slabelref && !lp->slabeldef)
	    die("statement label %s used but not defined\n",lp->node->u.string);
	else if (!lp->slabelref && lp->slabeldef)
	    warn("statement label %s defined but never used\n", lp->node->u.string);
        else if (lp->used && !lp->assigned)
	    warn("variable %s used but not set\n", lp->node->u.string);
        else if (!lp->used && lp->assigned)
	    warn("variable %s set but not used\n", lp->node->u.string);

    return(false);
}

static bool r_label_rewrite(node *tp)
/* resolve label references */
{
    if (ATOMIC(tp->type))
	return(true);

    /*
     * KISS -- this code may do extra work, but it needs no special
     * knowledge about node types.
     */
    if (BLABELREF(tp->type))
    {
	node *left = tp->car;
	node *right = tp->cdr;

	if (left && left->type == IDENTIFIER && left->syminf->blabelref)
	{
#ifdef PDEBUG
	    (void) printf("patching block label left reference to %s\n", left->u.string);
#endif /* PDEBUG */
	    tp->car = left->syminf->target;
	}
	if (right && right->type == IDENTIFIER && right->syminf->blabelref)
	{
#ifdef PDEBUG
	    (void) printf("patching block label right reference to %s\n", right->u.string);
#endif /* PDEBUG */
	    tp->cdr = right->syminf->target;
	}
    }

    if (SLABELREF(tp->type))
    {
	node *left = tp->car;
	node *right = tp->cdr;

	if (left && left->type == IDENTIFIER && left->syminf->slabelref)
	{
#ifdef PDEBUG
	    (void) printf("patching statement label left reference to %s\n", left->u.string);
#endif /* PDEBUG */
	    tp->car = left->syminf->target;
	}
	if (right && right->type == IDENTIFIER && right->syminf->slabelref)
	{
#ifdef PDEBUG
	    (void) printf("patching statement label right reference to %s\n", right->u.string);
#endif /* PDEBUG */
	    tp->cdr = right->syminf->target;
	}
    }

    /*
     * Remove all label nodes, now that we've resolved them.
     * NOTE: we must suppress this if we ever do a compiler back end!
     */
    if (tp->car && tp->car->type == LABEL)
    {
	node *oldnode = tp->car;
	tp->car = tp->car->cdr;
	free(oldnode);
    }

    return(true);
}

static void rewrite(node *tree)
/* resolve labels */
{
    node	*np;

    /* first, make a pointer from each label to its destination statement */
    for_cdr(np, tree)
	if (np->car->type == LABEL)
	    switch (np->car->cdr->type)
	    {
	    case END:
		break;

	    case BLOCK:
		/*
		 * NOTE: if we do a compiler back end, we probably want
		 * to suppress the cdr operation.
		 */
		np->car->car->syminf->target = np->cdr;
		break;

	    default:
		np->car->car->syminf->target = np;
		break;
	    }

    /* figure end addresses */
    for_cdr(np, tree)
    {
	node *sp = np->car;

	if (sp && sp->type == LABEL && sp->cdr->type == BLOCK)
	{
	    node	*ep;
	    bool	found = false;

	    for (ep = np->cdr; ep; ep = ep->cdr)
		if (ep->car->type == LABEL
			&& ep->car->cdr->type == END
			&& strcmp(sp->car->u.string,
				  ep->car->car->u.string) == 0)
		{
		    /*
		     * Associate the end node not with the BLOCK node
		     * itself but with the enclosing statement node.
		     * that way, we can find the first statement after
		     * the END by pc->endnode->cdr, where pc is the
		     * current statement.
		     */
		    np->endnode = ep;
		    found = true;
		    break;
		}
	    if (!found)
		die("no END matching %s BLOCK\n", sp->car->u.string);
	}
    }

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
