/* cupl.h -- basic types for the CUPL interpreter/compiler */

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

#ifndef TRUE
typedef int	bool;
#define TRUE	1
#define FALSE	0
#endif /* TRUE */

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

    /* back-pointer to the symbol list, set if the node is an identifier */
    struct lvar_t	*syminf;    

#ifdef PARSEDEBUG
    int number;		/* statement number, if this is a STATEMENT node */
#endif /* PARSEDEBUG */
}
node;

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
    node		*endnode;	/* end node address, if block label */

    /* information used for consistency checks */
    int		blabeldef;
    int		blabelref;
    int		slabeldef;
    int		slabelref;
    int		assigned;
    int		used;
}
lvar;
extern lvar *idlist;

#define for_symbols(s)    for (s = idlist; s; s = s->next)

#define NOMEM	"out of memory\n"

/* miscellaneous */
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
extern value cupl_log(value);
extern value cupl_sqrt(value);
extern value cupl_max(value, value);
extern value cupl_min(value, value);
extern value cupl_rand(value);

extern value cupl_sgm(value right);

extern bool cupl_eq(value, value);
extern bool cupl_lt(value, value);
extern bool cupl_gt(value, value);
extern bool cupl_le(value, value);
extern bool cupl_ge(value, value);

#define DEBUG_PARSEDUMP	1
#define DEBUG_CHECKDUMP	2
#define DEBUG_ALLOCATE	3

/* cupl.h ends here */
