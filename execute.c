/*****************************************************************************

NAME
   execute.c -- parse-tree execution

SYNOPSIS
   void execute(node *tree)	-- execute a parse tree

DESCRIPTION 
   This code does execution of a CUPL parse tree.  Actual execution is
handed off to execute().

*****************************************************************************/
/*LINTLIBRARY*/
#include <stdio.h>
#include <setjmp.h>
#include <math.h>
#include "cupl.h"
#include "tokens.h"
#include "nodetype.h"

#define car	u.n.left
#define cdr	u.n.right
#define EVAL_WRAP
#define RETURN_WRAP(t, l, r, v)	display_return(t, l, r, v);

static node *pc;	/* the statement node to evaluate next */
static node *data;	/* pointer to *DATA item to be grabbed next */
static jmp_buf jmpbuf;	/* termination handling */

static void cupl_read(node *tp)
/* evaluate a READ item */
{
    if (tp->type != IDENTIFIER)
	die("internal error -- garbled READ list\n");
    else
    {
	value *v = &(tp->syminf->value);
	int	n;

	for (n = 0; n < v->width * v->depth; n++)
	    if (data == (node *)NULL)
	    {
		warn("data list too short\n");
		v->elements[n] = 1;	/* 5-2 */
	    }
	    else
	    {
		if (data->car->type == NUMBER)
		    v->elements[n] = data->car->u.numval;
		else
		{
		    if (strcmp(tp->u.string , data->car->car->u.string))
			warn("data mismatch; expecting %s, saw %s\n",
			     tp->u.string , data->car->car->u.string);
		    v->elements[n] = data->car->cdr->u.numval;
		}

		data = data->cdr;
	    }
    }
}

static void eval_write(node *tp)
/* evaluate a WRITE item */
{
    if (tp == (node *)NULL)
	cupl_string_write("");
    else if (tp->type == ALL)
    {
	lvar	*lp;

	for_symbols(lp)
	    if (lp->used || lp->assigned)
		eval_write(lp->node);
    }
    else if (tp->type == STRING)
	cupl_string_write(tp->u.string);
    else if (tp->type == FWRITE)
	cupl_scalar_write((char *)NULL, tp->car->syminf->value.elements[0]);
    else
	cupl_scalar_write(tp->u.string, tp->syminf->value.elements[0]);
}

static void display_return(node *tree, node *left, node *right, value v)
/* display the returned value of a node (for tracing purposes) */
{
    if (verbose >= DEBUG_ALLOCATE)
    {
	(void) printf("eval: %x (%-10s of %9x, %9x) ",
		      tree, tokdump(tree->type), left, right);

	if (v.rank == FAIL)
	    (void) printf("does not return a value\n");
	else
	    (void) printf("returned %f\n", v.elements[0]);
    }
}

static value cupl_eval(node *tree)
/* recursively evaluate a CUPL parse tree */
{
    value	leftside, rightside, result;
    node	*np;

    switch(tree->type)
    {
    case NUMBER:
	make_scalar(&result, tree->u.numval);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case IDENTIFIER:
	result = copy_value(tree->syminf->value);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case READ:
	for_cdr(np, tree)
	    cupl_read(np->car);
	result.rank = FAIL;
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case WRITE:
	cupl_reset_write(0);
	for_cdr(np, tree)
	    eval_write(np->car);
	cupl_eol_write();
	result.rank = FAIL;
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case LET:
	deallocate_value(&(tree->car->syminf->value));
	tree->car->syminf->value = EVAL_WRAP(cupl_eval(tree->cdr));
	result.rank = FAIL;
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case PLUS:
	leftside = EVAL_WRAP(cupl_eval(tree->car));
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_add(leftside, rightside);
	deallocate_value(&leftside); deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case MULTIPLY:
	leftside = EVAL_WRAP(cupl_eval(tree->car));
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_multiply(leftside, rightside);
	deallocate_value(&leftside); deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case MINUS:
	leftside = EVAL_WRAP(cupl_eval(tree->car));
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_subtract(leftside, rightside);
	deallocate_value(&leftside); deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case DIVIDE:
	leftside = EVAL_WRAP(cupl_eval(tree->car));
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_divide(leftside, rightside);
	deallocate_value(&leftside); deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case POWER:
	leftside = EVAL_WRAP(cupl_eval(tree->car));
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_power(leftside, rightside);
	deallocate_value(&leftside); deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case UMINUS:
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_uminus(rightside);
	deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case ABS:
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_abs(rightside);
	deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case ATAN:
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_atan(rightside);
	deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case COS:
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_cos(rightside);
	deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case EXP:
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_exp(rightside);
	deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case FLOOR:
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_floor(rightside);
	deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case LOG:
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_log(rightside);
	deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case SQRT:
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_sqrt(rightside);
	deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case MAX:
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_max(leftside, rightside);
	deallocate_value(&leftside);
	deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case MIN:
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_min(leftside, rightside);
	deallocate_value(&leftside);
	deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case RAND:
	rightside = EVAL_WRAP(cupl_eval(tree->cdr));
	result = cupl_rand(rightside);
	deallocate_value(&rightside);
	RETURN_WRAP(tree, tree->car, tree->cdr, result)
	return(result);

    case STOP:
	longjmp(jmpbuf, 1);
	/* no fall through */

    default:
	die("unknown node type %d (%s), cannot execute\n",
	    tree->type,
	    tokdump(tree->type));
	break;
    }
}

void execute(node *tree)
/* execute a CUPL program described by a parse tree */
{
    node	*np, *next, *last;
    lvar	*lp;

    /* initially, all variables are scalars with zero values */
    for_symbols(lp)
	make_scalar(&lp->value, 0);

    /* locate the data pointer */
    data = last = (node *)NULL;
    for_cdr(np, tree)
    {
	if (np->car->type == DATA)
	{
	    data = np->car;
	    break;
	}
	last = np;
    }

    /* break the link to the data */
    if (!data)
	warn("no data supplied\n");
    else if (last)
	last->cdr = (node *)NULL;

    /* first, setjmp so we can use STOP to exit */
    if (setjmp(jmpbuf) != 0)
	return;
    else
	/* now execute the program */
	for (pc = tree; pc; pc = next)
	{
	    next = pc->cdr;
	    (void) cupl_eval(pc->car);
	}

    warn("program terminated without explicit STOP\n");
}

/* execute.c ends here */
