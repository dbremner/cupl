/*****************************************************************************

NAME
   monitor.c -- parse-tree execution

SYNOPSIS
    void die(char *msg, ...)
    void warn(char *msg, ...)

    void make_scalar(value *v, scalar i)
    void copy_value(value v);
    value allocate_value(int rank, int i, int j)
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
    value cupl_ln(value)
    value cupl_log(value)
    value cupl_sqrt(value)
    value cupl_max(value, value)
    value cupl_min(value, value)
    value cupl_rand(value)

    bool cupl_eq(value, value)
    bool cupl_lt(value, value)
    bool cupl_gt(value, value)
    bool cupl_le(value, value)
    bool cupl_ge(value, value)

    value cupl_det(value);
    value cupl_dot(value, vlue);
    value cupl_inv(value);
    value cupl_posmax(value);
    value cupl_posmin(value);
    value cupl_sgm(value right)
    value cupl_trc(value right)
    value cupl_trn(value right)

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

value allocate_value(int rank, int i, int j)
/* allocate a value of given shape */
{
    value v;

    v.rank = rank;
    v.width = j; v.depth = i;
    v.elements = (scalar *)calloc(sizeof(scalar), i * j);

    return(v);
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
    (void) putchar('\n');
    used = 0;
}

static void needspace(int w)
/* emit a LF if there are not more than w spaces left on the line */
{
    used += w;
    if (used >= linewidth)
    {
	cupl_eol_write();
	used = w;
    }
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

    if (0.001 < fabs(quant) && fabs(quant) < 100000)
	(void) printf("%*.9f", fieldwidth, quant);
    else
	(void) printf("%*.9E", fieldwidth, quant);
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
	die("addition failed, operands of different sizes or ranks\n");
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
	die("subtract failed, operands of different sizes or ranks\n");
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
    else if (left.width == right.depth)
    {
	value	result;
	int	i, j;

	result = allocate_value(2, left.depth, right.width);
	for (i = 0; i < left.depth; i++)
	    for (j = 0; j < right.width; j++)
	    {
		int k;
		scalar p = 0;

		for (k = 0; k < left.width; k++)
		    p += SUB(left, i, k)[0] * SUB(right, k, j)[0];

		SUB(result, i, j)[0] = p;
	    }
	return(result);
    }
    else
	die("multiplication attempt on non-conformable matrices\n");
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
    else if (right.rank == 0)
    {
	value	result;
	int	n;

	result = copy_value(left);
	for (n = 0; n < left.width * left.depth; n++)
	    result.elements[n] = left.elements[n] / right.elements[0];
	return(result);
    }
    else
	die("division of rank %d by rank %d value is undefined\n",
	    left.rank, right.rank);
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
    int	n;

    result = copy_value(right);
    for (n = 0; n < right.width * right.depth; n++)
	result.elements[n] = fabs(right.elements[n]);
    return(result);
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

value cupl_ln(value right)
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

value cupl_log(value right)
/* apply log10 function */
{
    value	result;

    if (right.rank != 0)
	die("LOG is only defined for scalar arguments\n");
    else
    {
	make_scalar(&result, 0);
	result.elements[0] = log10(right.elements[0]);
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
    int	n;

    make_scalar(&result, right.elements[0]);
    for (n = 0; n < right.width * right.depth; n++)
	if (result.elements[0] < right.elements[n])
	    result.elements[0] = right.elements[n];

    return(result);
}

value cupl_min(value left, value right)
/* apply min functionnot yet supported for vectors and matrices */
{
    value	result;
    int	n;

    make_scalar(&result, right.elements[0]);
    for (n = 0; n < right.width * right.depth; n++)
	if (result.elements[0] > right.elements[n])
	    result.elements[0] = right.elements[n];

    return(result);
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
 * Relations
 *
 ****************************************************************************/

/*
 * Original CUPL's roundoff rule for relations seems to have been designed
 * to throw away all digits of precision more than 14,
 */
#define FUZZY_EQUAL(m, n)	(fabs((m) - (n)) < 10e-15)

bool cupl_eq(value v1, value v2)
/* test any two CUPL values for pairwise equality */
{
    if (!CONGRUENT(v1, v2))
	die("comparison failed, operands of different sizes or ranks\n");
    else
    {
	int	n;

	for (n = 0; n < v2.width * v2.depth; n++)
	{
	    scalar e1 = v1.elements[n], e2 = v2.elements[n];

	    if (!FUZZY_EQUAL(e1, e2))
		return(FALSE);
	}

	return(TRUE);
    }
}

bool cupl_le(value v1, value v2)
/* are all elements of v1 less than or equal to their correspondents in v2? */
{
    if (!CONGRUENT(v1, v2))
	die("LE failed, operands of different sizes or ranks\n");
    else
    {
	bool equal = TRUE;
	int	n;


	for (n = 0; n < v2.width * v2.depth; n++)
	{
	    scalar e1 = v1.elements[n], e2 = v2.elements[n];

	    if (!FUZZY_EQUAL(e1, e2) && e1 > e2)
		return(FALSE);
	}

	return(TRUE);
    }
}

bool cupl_ge(value v1, value v2)
/* are all els of v1 greater than or equal to their correspondents in v2? */
{
    if (!CONGRUENT(v1, v2))
	die("GE failed, operands of different sizes or ranks\n");
    else
    {
	int	n;

	for (n = 0; n < v2.width * v2.depth; n++)
	{
	    scalar e1 = v1.elements[n], e2 = v2.elements[n];

	    if (!FUZZY_EQUAL(e1, e2) && e1 < e2)
		return(FALSE);
	}

	return(TRUE);
    }
}

bool cupl_lt(value v1, value v2)
/* strange CUPL definition #1, see appendix A */
{
    return(cupl_le(v1, v2) && !cupl_eq(v1, v2));
}

bool cupl_gt(value v1, value v2)
/* strange CUPL definition #2, see appendix A */
{
    return(cupl_ge(v1, v2) && !cupl_eq(v1, v2));
}

/****************************************************************************
 *
 * Matrix functions
 *
 ****************************************************************************/

value cupl_det(value right)
{
    /* FIXME: implement DET */
    die("the determinant function is not yet implemented");
}

value cupl_dot(value left, value right)
/* compute inner or dot product of two vectors */
{
    if (!CONGRUENT(left, right) || right.rank != 1)
	die("DOT failed, operands of different sizes or ranks\n");
    else
    {
	value	result;
	int	n;

	make_scalar(&result, 0);
	for (n = 0; n < left.width * left.depth; n++)
	    result.elements[0] += left.elements[n] * right.elements[n];
	return(result);
    }
}

value cupl_inv(value right)
/* compute the inverse of a matrix */
{
    /* FIXME: implement INV */
    die("the inverse function is not yet implemented\n");
}

value cupl_posmax(value right)
/* row position of maximum element of argument */
{
    if (right.rank == 0)
	die("POSMAX argument is a scalar");
    else
    {
	value	result;
	int	n;
	scalar	maxel = right.elements[0];;

	make_scalar(&result, 0);
	for (n = 0; n < right.width * right.depth; n++)
	    if (maxel < right.elements[n])
	    {
		result.elements[0] = SUBI(right, n);
		maxel = right.elements[0];
	    }

	return(result);
    }
}

value cupl_posmin(value right)
/* row position of minimum element of argument */
{
    if (right.rank == 0)
	die("POSMIN argument is a scalar");
    else
    {
	value	result;
	int	n;
	scalar	minel = right.elements[0];;

	make_scalar(&result, 0);
	for (n = 0; n < right.width * right.depth; n++)
	    if (minel > right.elements[n])
	    {
		result.elements[0] = SUBI(right, n);
		minel = right.elements[n];
	    }

	return(result);
    }
}

value cupl_sgm(value right)
/* compute the sum of the elements of a matrix */
{
    value result;
    int	n;

    make_scalar(&result, 0);
    for (n = 0; n < right.width * right.depth; n++)
	result.elements[0] += fabs(right.elements[n]);
    return(result);
}

value cupl_trc(value right)
/* compute sum of elements on main diagonals */
{
    if (right.rank != 2 || right.width != right.depth)
	die("TRACE failed, operands is not a square matrix\n");
    else
    {
	value	result;
	int	n;

	make_scalar(&result, 0);
	for (n = 0; n < right.width; n++)
	    result.elements[0] += SUB(right, n, n)[0];
	return(result);
    }
}

value cupl_trn(value right)
/* compute the transpose of a matrix */
{
    value result = allocate_value(right.rank, right.depth, right.width);
    int	i, j;

    for (i = 0; i < right.depth; i++)
	for (j = 0; j < right.width; j++)
	    SUB(result, j, i)[0] = SUB(right, i, j)[0];

    return(result);
}

/* monitor.c ends here */
