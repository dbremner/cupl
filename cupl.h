/* cupl.h -- basic types for the CUPL interpreter/compiler */

/* SPDX-License-Identifier: BSD-2-clause */

/* maximum file size */
#ifdef USG
#include <limits.h>
#else
#define PATH_MAX	1024
#endif

#ifndef CUPLDIR
#define CUPLDIR	"/usr/lib/cupl/"
#endif /* CUPLDIR */

#define EXT	".cupl"		/* CUPL source file extension */

#include <stdbool.h>

#define SUCCEED	0
#define FAIL	-1

/*
 * Type used by cupl for representing a CUPL scalar. 
 * Should be a floating-point type, e.g. float, double, long float, etc.
 * Also, you must define here the function used to read scalars.
 */
typedef double scalar;
#define atos(s)	atof(s)

/*
 * This is the node structure for parse trees.
 */
typedef struct edon
{
    /* type of the node -- a YACC token type */
    int		type;

    /* value elements of the node */
    union
    {
	struct			/* for parse-tree nodes */
	{
	    struct edon	*left;
	    struct edon	*right;
	} n;

	scalar	numval;		/* for numbers */

	char	*string;	/* for strings and identifiers */
    } u;

    /* identifier nodes only, back-pointer to the symbol list */
    struct lvar_t	*syminf;    

    /* statement nodes only */
    struct edon		*endnode;	/* end node address, if block label */
#ifdef PARSEDEBUG
    int 		number;		/* statement number */
#endif /* PARSEDEBUG */
}
node;

#define	NULLNODE	(node *)NULL

#define car	u.n.left
#define cdr	u.n.right

#define for_cdr(x, t)    for (x = (t); x; x = x->u.n.right)

/* this structure represents a CUPL value */
typedef struct
{
    int		rank;			/* 0, 1, or 2 */
    int		width, depth;		/* dimensions */
    scalar	*elements;		/* elements */
}
value;

/* access to the symbol list */
typedef struct lvar_t
{
    struct lvar_t	*next;		/* link to next variable */
    node		*node;		/* variable's symbol info */
    value		value;		/* variable's value */
    node		*target;	/* target node, if label */

    /* information used for consistency checks */
    int		blabeldef;
    int		blabelref;
    int		slabeldef;
    int		slabelref;
    int		assigned;
    int		used;
    int		watchcount;
}
lvar;
extern lvar *idlist;

bool corc;	/* are we parsing CUPL or CORC? */ 

#define for_symbols(s)    for (s = idlist; s; s = s->next)

/* subscripting operations */
#define SUB(v, i, j)	(v.elements + i * v.width + j)
#define SUBI(v, n)	(n / v.width)
#define SUBJ(v, n)	(n % v.width)

#define NOMEM	"out of memory\n"

/* miscellaneous */
extern node *cons(int, node *, node *);
extern char *tokdump(int value);
extern void yyerror(const char *errmsg);
extern void interpret(node *tree);
extern int verbose, linewidth, fieldwidth;

/* execute.c */
extern void execute(node *tree);

/* monitor.c */
extern void die(char *msg, ...);
extern void warn(char *msg, ...);

extern void make_scalar(value *v, scalar n);
extern value copy_value(value);
extern value allocate_value(int rank, int i, int j);
extern void deallocate_value(value *);

void cupl_reset_write();
void cupl_eol_write();
void cupl_scalar_write(char *name, scalar quant);
void cupl_string_write(char *s);

extern value cupl_add(value, value);
extern value cupl_multiply(value, value);
extern value cupl_subtract(value, value);
extern value cupl_divide(value, value);
extern value cupl_power(value, value);
extern value cupl_uminus(value);

extern value cupl_abs(value);
extern value cupl_atan(value);
extern value cupl_cos(value);
extern value cupl_exp(value);
extern value cupl_floor(value);
extern value cupl_ln(value);
extern value cupl_log(value);
extern value cupl_sqrt(value);
extern value cupl_max(value, value);
extern value cupl_min(value, value);
extern value cupl_rand(value);

extern value cupl_det(value);
extern value cupl_dot(value, value);
extern value cupl_inv(value);
extern value cupl_posmax(value);
extern value cupl_posmin(value);
extern value cupl_sgm(value);
extern value cupl_trc(value);
extern value cupl_trn(value);

extern bool cupl_eq(value, value);
extern bool cupl_lt(value, value);
extern bool cupl_gt(value, value);
extern bool cupl_le(value, value);
extern bool cupl_ge(value, value);

#define DEBUG_PARSEDUMP	1
#define DEBUG_CHECKDUMP	2
#define DEBUG_EXECUTE	3
#define DEBUG_ALLOCATE	4

/* cupl.h ends here */
