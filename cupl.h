/* cupl.h -- basic types for the CUPL interpreter/compiler */

/* maximum file size */
#ifdef USG
#include <limits.h>
#else
#define PATH_MAX	1024
#endif

#define MAXPERFORM	64	/* depth of perform stack */
#define MAXVARS		1024	/* maximum variables */
#define MAXNAME		10	/* maximum length of variable names */
#define MAXSTR		256	/* maximum string size */

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
    int		type;	/* type of NODE if *left || *right */
    union
    {
	struct			/* for parse-tree nodes */
	{
	    struct edon	*left;
	    struct edon	*right;
	} n;

	scalar	numval;		/* for numbers */

	char	*string;	/* for string literals */

	struct			/* for label values */
	{
	    struct edon *dest;
	    char	*string;
	} l;
    } u;
}
node;

/* access to the symbol list */
typedef struct lvar_t
{
    struct lvar_t	*next;
    int			refcount;
    int			setcount;
    bool		islabel;
    node		*node;
}
lvar;
static lvar *idlist;

#define NOMEM	"out of memory\n"

extern char *tokdump(int value);
extern void yyerror(const char *errmsg);
extern void interpret(node *tree);
extern int verbose;

/* cupl.h ends here */
