/*****************************************************************************

NAME
   monitor.c -- parse-tree execution

SYNOPSIS
    void die(char *msg, ...)
    void warn(char *msg, ...)

    void make_scalar(value *v, scalar i)
    void copy_value(value v);
    void deallocate_value(value *v)

    void cupl_reset_write()
    void cupl_eol_write()
    void cupl_scalar_write(char *name, scalar quant)
    void cupl_string_write(char *s)

    value cupl_add(value, value)
    value cupl_multiply(value, value)
    value cupl_subtract(value, value)
    value cupl_divide(value, value)
    value cupl_uminus(value, value)

    value cupl_abs(value)
    value cupl_atan(value)
    value cupl_cos(value)
    value cupl_exp(value)
    value cupl_floor(value)
    value cupl_log(value)
    value cupl_sqrt(value)
    value cupl_max(value, value)
    value cupl_min(value, value)
    value cupl_rand(value)

DESCRIPTION 
   Runtime support.  This is segregated from the execute() code in case anyone
ever wants to write a back end that is a compiler.  For the same reason
we do an allocate each time an intrinsic returns a value.  This is
relatively inefficient but means these functions could be used as a runtime
library.

*****************************************************************************/
/*LINTLIBRARY*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include "cupl.h"

#define max(x, y)	((x) > (y) ? (x) : (y))
#define min(x, y)	((x) < (y) ? (x) : (y))

/****************************************************************************
 *
 * Error reporting
 *
 ****************************************************************************/

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

/****************************************************************************
 *
 * Value allocation
 *
 ****************************************************************************/

void make_scalar(value *v, scalar i)
/* initialize a scalar value element */
{
    v->rank = 0;
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

/****************************************************************************
 *
 * Output support
 *
 ****************************************************************************/

static int used;

void cupl_reset_write()
{
    used = 0;
}

void cupl_eol_write(void)
{
    if (used != linewidth)
	(void) putchar('\n');
}

static void needspace(int w)
/* emit a LF if there are not more than w spaces left on the line */
{
    if (used + w >= linewidth)
    {
	(void) putchar('\n');
	used = 0;
    }
    else
	used += w;
}

void cupl_scalar_write(char *name, scalar quant)
/* write a numeric or skip a field in CUPL style */
{
    if (name)
    {
	needspace(2 * fieldwidth);
	(void) printf("%*s = ", fieldwidth - 3, name);
    }
    else
	needspace(fieldwidth);

    if (0.001 < fabs(quant) && abs(quant) < 100000)
	(void) printf("%*f", fieldwidth, quant);
    else
	(void) printf("%*E", fieldwidth, quant);
}

void cupl_string_write(char *s)
/* write a string, or just skip the field */
{
    needspace(fieldwidth);
    (void) printf("%-*s", fieldwidth, s);
}

/****************************************************************************
 *
 * Functions for arithmetic intrinsics
 *
 ****************************************************************************/

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

/****************************************************************************
 *
 * Scalar functions
 *
 ****************************************************************************/

value cupl_abs(value right)
/* apply absolute-value function */
{
    value	result;

    if (right.rank != 0)
	die("ABS is only defined for scalar arguments\n");
    else
    {
	make_scalar(&result, 0);
	result.elements[0] = abs(right.elements[0]);
	return(result);
    }
}

value cupl_atan(value right)
/* apply arctangent */
{
    value	result;

    if (right.rank != 0)
	die("ATAN is only defined for scalar arguments\n");
    else
    {
	make_scalar(&result, 0);
	result.elements[0] = atan(right.elements[0]);
	return(result);
    }
}

value cupl_cos(value right)
/* apply cosine */
{
    value	result;

    if (right.rank != 0)
	die("COS is only defined for scalar arguments\n");
    else
    {
	make_scalar(&result, 0);
	result.elements[0] = cos(right.elements[0]);
	return(result);
    }
}

value cupl_exp(value right)
/* apply exponent function */
{
    value	result;

    if (right.rank != 0)
	die("EXP is only defined for scalar arguments\n");
    else
    {
	make_scalar(&result, 0);
	result.elements[0] = exp(right.elements[0]);
	return(result);
    }
}

value cupl_floor(value right)
/* apply floor function */
{
    value	result;

    if (right.rank != 0)
	die("FLOOR is only defined for scalar arguments\n");
    else
    {
	make_scalar(&result, 0);
	result.elements[0] = floor(right.elements[0]);
	return(result);
    }
}

value cupl_log(value right)
/* apply ln function */
{
    value	result;

    if (right.rank != 0)
	die("LOG is only defined for scalar arguments\n");
    else
    {
	make_scalar(&result, 0);
	result.elements[0] = log(right.elements[0]);
	return(result);
    }
}

value cupl_sqrt(value right)
/* apply square root */
{
    value	result;

    if (right.rank != 0)
	die("SQRT is only defined for scalar arguments\n");
    else
    {
	make_scalar(&result, 0);
	result.elements[0] = sqrt(right.elements[0]);
	return(result);
    }
}

/****************************************************************************
 *
 * Special functions
 *
 ****************************************************************************/

value cupl_max(value left, value right)
/* apply max function */
{
    value	result;

    if (right.rank != 0 || left.rank != 0)
	die("MAX is not yet supported for vectors and matrices\n");
    else
    {
	make_scalar(&result, 0);
	result.elements[0] = max(right.elements[0], left.elements[0]);
	return(result);
    }
}

value cupl_min(value left, value right)
/* apply min functionnot yet supported for vectors and matrices */
{
    value	result;

    if (right.rank != 0 || left.rank != 0)
	die("MIN is \n");
    else
    {
	make_scalar(&result, 0);
	result.elements[0] = min(right.elements[0], left.elements[0]);
	return(result);
    }
}

value cupl_rand(value right)
/* get a random number from a seed */
{
    value	result;

    if (right.rank != 0)
	die("RAND is only defined for scalar arguments\n");
    else
    {
	make_scalar(&result, 0);
	srand(right.elements[0]);
	result.elements[0] = rand();
	return(result);
    }
}

/****************************************************************************
 *
 * Matrix functions
 *
 ****************************************************************************/

/* FIXME: not yet implemented */

/* monitor.c ends here */
