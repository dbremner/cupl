/*****************************************************************************

NAME
   monitor.c -- parse-tree execution

SYNOPSIS
    void die(char *msg, ...)

    void warn(char *msg, ...)

    void make_scalar(value *v, scalar i)

    void copy_value(value v);
    void deallocate_value(value *v)

    value cupl_add(value, value)
    value cupl_multiply(value, value)
    value cupl_subtract(value, value)
    value cupl_divide(value, value)
    value cupl_uminus(value, value)
    value cupl_sqrt(value, value)

DESCRIPTION 
   Runtime support.  This is segregated from the execute() code in case anyone
ever wants to write a back end that is a compiler.

*****************************************************************************/
/*LINTLIBRARY*/
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "cupl.h"

void warn(char *msg, ...)
/* warn of an error and die */
{
    va_list	args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
}

void die(char *msg, ...)
/* complain of a fatal error and die */
{
    va_list	args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    exit(1);
}

void make_scalar(value *v, scalar i)
/* initialize a scalar value element */
{
    v->width = v->depth = 1;
    v->elements = (scalar *)malloc(sizeof(scalar));
    v->elements[0] = i;
}

value copy_value(value v)
/* male a new copy of a value element */
{
    value	newvalue = v;

    newvalue.elements = (scalar *)malloc(sizeof(scalar) * v.width * v.depth);
    memcpy(newvalue.elements, v.elements,
	   sizeof(scalar) * v.width * v.depth);
    return(newvalue);
}

void deallocate_value(value *v)
/* destroy a value copy, only if its reference count is 1 */
{
    (void) free(v->elements);
}

#define CONGRUENT(l, r)	((l.rank == r.rank) \
				&& (l.width == r.width) \
				&& (l.depth == r.depth))

value cupl_add(value left, value right)
/* add two CUPL values */
{
    if (!CONGRUENT(left, right))
	die("addition failed, operands of different sizes\n");
    else
    {
	value	result;
	int	n;

	result = copy_value(right);
	for (n = 0; n < left.width * left.depth; n++)
	    result.elements[n] = left.elements[n] + right.elements[n];
	return(result);
    }
}

value cupl_subtract(value left, value right)
/* subtract two CUPL values */
{
    if (!CONGRUENT(left, right))
	die("subtract failed, operands of different sizes\n");
    else
    {
	value	result;
	int	n;

	result = copy_value(right);
	for (n = 0; n < left.width * left.depth; n++)
	    result.elements[n] = left.elements[n] - right.elements[n];
	return(result);
    }
}

value cupl_multiply(value left, value right)
/* multiply two CUPL values */
{
    if (left.rank == 0 && right.rank == 0)
    {
	value	result;

	make_scalar(&result, 0);
	result.elements[0] = left.elements[0] * right.elements[0];
	return(result);
    }
    else
	die("multiplication of non-scalars is not yet supported\n");
}

value cupl_divide(value left, value right)
/* divide two CUPL values */
{
    if (left.rank == 0 && right.rank == 0)
    {
	value	result;

	make_scalar(&result, 0);
	result.elements[0] = left.elements[0] / right.elements[0];
	return(result);
    }
    else
	die("division of non-scalars is not yet supported\n");
}

value cupl_power(value left, value right)
/* apply power operation with two CUPL values */
{
    if (left.rank == 0 && right.rank == 0)
    {
	value	result;

	make_scalar(&result, 0);
	result.elements[0] = pow(left.elements[0], right.elements[0]);
    }
    else
	die("power operation on non-scalars is not yet supported\n");
}

value cupl_uminus(value right)
/* apply unary minus */
{
    int	n;
    value	result;

    result = copy_value(right);
    for (n = 0; n < right.width * right.depth; n++)
	result.elements[n] = -result.elements[n];
    return(result);
}

value cupl_sqrt(value right)
/* apply square root */
{
    value	result;

    if (right.rank != 0)
	die("SQRT is only defined for scalars\n");
    else
    {
	make_scalar(&result, 0);
	result.elements[0] = sqrt(right.elements[0]);
	return(result);
    }
}

/* monitor.c ends here */
