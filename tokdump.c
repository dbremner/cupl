/* tokdump.c -- a token dumper for debugging */

#include <string.h>
#include "tokens.h"

typedef struct
{
    int	value;
    char *name;
}
symbol;

static symbol toknames[] =
{
#include "toktab.h"
};

char *tokdump(int value)
{
    symbol	*sym;

    if (value < 128)
    {
	static char namestring[2];

	namestring[0] = value;
	return(namestring);
    }

    for (sym = toknames; sym < toknames+sizeof(toknames)/sizeof(symbol); sym++)
	if (sym->value == value)
	{
	    return(sym->name);
	    break;
	}

    return((char *)NULL);
}

/* tokdump.c ends here */
 

