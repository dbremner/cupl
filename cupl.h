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
}
node;

#define for_cdr(x, t)    for (x = (t); x; x = x->u.n.right)

/* this structure represents a CUPL value */
typedef struct
{
    int		type;			/* type (syntax class) */
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
    int		labeldef;
    int		labelref;
    int		assigned;
    int		used;
}
lvar;
extern lvar *idlist;

#define for_symbols(s)    for (s = idlist; s; s = s->next)

#define NOMEM	"out of memory\n"

extern char *tokdump(int value);
extern void yyerror(const char *errmsg);
extern void interpret(node *tree);
extern void execute(node *tree);
extern int verbose;

#define DEBUG_PARSEDUMP	1
#define DEBUG_CHECKDUMP	2
#define DEBUG_ALLOCATE	3

/* cupl.h ends here */
