/* nodetype.h -- macros that describe the semantics of nodes */
/* SPDX-License-Identifier: BSD-2-clause */

#define car	u.n.left
#define cdr	u.n.right

/*
 * Define token classes for consistency checks here.  The theory is
 * that by separating this from the tree-traversal machinery below, we
 * may be able to parametrize for different source languages someday.
 */
/* does the node represent an atom? */
#define ATOMIC(n)	((n) == IDENTIFIER || (n) == NUMBER || (n) == STRING)
/* does the node reference a statement label? */
#define SLABELREF(n)	((n) == GO || (n) == OG)
/* does the node reference a block label? */
#define BLABELREF(n)	((n) == PERFORM)
/* does the node set a variable? */
#define VARSET(n)	((n) == LET || (n) == READ || (n) == ITERATE)
/* does the (non-VARSET) node refer to its left operand as a variable? */ 
#define LEFTREF(n)	((n) != FOR && (n) != GO && (n) != OG && (n) != LABEL \
				&& (n) != ALLOCATE && (n) != WATCH \
				&& (n) != ITERATE && (n) != FROM \
		     		&& (n) != PERFORM)
/* does the (non-VARSET) node refer to its right operand? */
#define RIGHTREF(n)	((n) != PERFORM)

/* nodetype.h ends here */
