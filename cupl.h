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

	double	numval;		/* for numbers */

	char	*string;	/* for string literals */

	struct			/* for label values */
	{
	    struct edon *dest;
	    char	*string;
	} l;
    } u;
}
node;

#define NOMEM	"out of memory\n"

extern char *tokdump(int value);
extern void yyerror(const char *errmsg);
extern void interpret(node *tree);
extern int verbose;

/* cupl.h ends here */
