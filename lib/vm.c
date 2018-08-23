#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "vm.h"
#include "state.h"
#include "value.h"
#include "lexer.h"
#include "token.h"
#include "opcode.h"
#include "diagnostic.h"

/* 
 * carries out the specified operation on
 * the specified operand _a and _b which are in
 * the type of `struct value *`, as floats
 */
#define BINOP_REAL(_op, _a, _b, _o) 						\
		(_o)->type = VALUE_TYPE_REAL;						\
		(_o)->as_float = (_a)->as_float _op (_b)->as_float;	\

/* 
 * carries out the specified operation on
 * the specified operand _a and _b which are in
 * the type of `struct value *`, as ints
 */
#define BINOP_INTEGER(_op, _a, _b, _o) 						\
		(_o)->type = VALUE_TYPE_INTEGER;					\
		(_o)->as_int = (_a)->as_int _op (_b)->as_int;		\

/*
 * ensures that there is enough
 * stack space for the specified amount
 * of stack values
 */
static inline bool stack_ensure(struct state *state, size_t size) {
	/* check if we exceed the max stack size */
	if (size >= state->config->max_stack_size) {
		return false;
	}

	/* calculate index of sp */
	size_t cur = (size_t)(state->sp - state->stack);
	/* calculate the current size of the stack */
	size_t cur_size = (size_t)(state->stack_top - state->stack);

	/* check if we need to grow the stack */
	if (size > cur_size) {
		struct value *stack;

		stack = state->stack;
		stack = realloc(state->stack, sizeof(struct value) * size);

		/* check if realloc failed */
		if (stack == NULL) { 
			return false;
		}

		state->stack = stack;
		state->stack_top = state->stack + size;
		state->sp = state->stack + cur;
		return true;
	}

	return true;
}

/*
 * pops a value from the top of the
 * stack of the virtual machine
 */
static inline struct value *stack_pop(struct state *state) {
	return --state->sp;
}

/*
 * pushes a value on top of the
 * stack of the virtual machine
 */
static inline void stack_push(struct state *state, struct value *val) {
	*state->sp++ = *val;
}

/* adds an error the list of errors in the specified state */
static inline void error(struct state *state, struct diagnostic *err) {
	/* space */
}

/* outputs the specified value */
static void output(struct value *value) {
	switch (value->type) {
		case VALUE_TYPE_BOOLEAN:
			printf(value->as_bool ? "TRUE\n" : "FALSE\n");
			break;
		case VALUE_TYPE_INTEGER:
			printf("%d\n", value->as_int);
			break;
		case VALUE_TYPE_REAL:
			printf("%.f\n", value->as_float);
			break;
		case VALUE_TYPE_POINTER:
			printf("pointer<%p>\n", value->as_pointer);
			break;
		case VALUE_TYPE_OBJECT:
			if (value->as_object == NULL) {
				printf("object<NULL>\n");
			} else {
				/* TODO: handle arrays as well */
				if (value->as_object->type == &string_type) {
					struct string_object *string = (struct string_object *)value->as_object;
					printf("%s\n", string->buffer);
				} else {
					printf("object<%p>\n", value->as_object);
				}
			}
			break;
		default:
			printf("unknown(%d)<%p>\n", value->type, value->as_object);
			break;
	}
}

/* converts a value of type object which is a string_object to */
static int string_to_number(struct value *a, struct value *result) {
	/*
	 * NOTE: probably losing some perf here, since we're using a lexer
	 * to determine the number type then use the std functions which itself
	 * lexes the string again
	 */
	struct lexer lexer;
	struct token token;
	struct string_object *str;

	str = (struct string_object *)a->as_object;
	lexer_init(&lexer, str->buffer);
	lexer_lex(&lexer, &token);

	/* convert to the appropriate type */
	switch (token.type) {
		case TOK_LIT_INTEGER:
			result->type = VALUE_TYPE_INTEGER;
			result->as_int = strtol(str->buffer, NULL, 10);
			break;
		case TOK_LIT_INTEGERHEX:
			result->type = VALUE_TYPE_INTEGER;
			result->as_int = strtol(str->buffer, NULL, 16);
			break;
		case TOK_LIT_REAL:
			result->type = VALUE_TYPE_REAL;
			result->as_float = strtof(str->buffer, NULL);
			break;

		default:
			/* non number literal token */
			return 1;
	}

	/*
	 * expect the next token to be EOF, which means
	 * we are strict on the coercions
	 *
	 * so things like this "12 oixD" is not coerced
	 * to 12
	 */
	lexer_lex(&lexer, &token);
	if (token.type != TOK_EOF) {
		return 1;
	}

	return 0;
}

/* converts an integer value type to a real value type */
static inline void integer_to_real(struct value *a) {
	a->type = VALUE_TYPE_REAL;
	a->as_float = (float)a->as_int;
}

/* carries out arithmetic operations on real values */
static int arith_real(struct state *state, enum vm_op op, struct value *a, struct value *b, struct value *result) {
	switch (op) {
		case VM_OP_ADD:
			BINOP_REAL(+, a, b, result);
			return 0;
		case VM_OP_SUB:
			BINOP_REAL(-, a, b, result);
			return 0;
		case VM_OP_MUL:
			BINOP_REAL(*, a, b, result);
			return 0;
		case VM_OP_DIV:
			/* prevent divided by 0s */
			if (b->as_float == 0) {
				error(state, NULL);
				return 1;
			}

			BINOP_REAL(/, a, b, result);
			return 0;

		default:
			return 1;
	}
}

/* carries out arithmetic operations on integer values */
static int arith_integer(struct state *state, enum vm_op op, struct value *a, struct value *b, struct value *result) {	
	switch (op) {
		case VM_OP_ADD:
			BINOP_INTEGER(+, a, b, result);
			return 0;
		case VM_OP_SUB:
			BINOP_INTEGER(-, a, b, result);
			return 0;
		case VM_OP_MUL:
			BINOP_INTEGER(*, a, b, result);
			return 0;
		case VM_OP_DIV:
			/* prevent divided by 0s */
			if (b->as_int == 0) {
				return 1;
			}

			BINOP_INTEGER(/, a, b, result);
			return 0;

		default:
			return 1;
	}
}

/* coerce values and carries out arithemtic operations on them */
static int arith_coerce(struct state *state, enum vm_op op, struct value *a, struct value *b, struct value *result) {
	/* for when we convert types */
	struct value na;
	struct value nb;

	/* if a is a string, convert it to a number */
	if (value_is_string(a)) {
		string_to_number(a, &na);
		a = &na;
	}

	/* if b is a string, convert it to a number */
	if (value_is_string(b)) {
		string_to_number(b, &nb);
		b = &nb;
	}
	
	/* check if both a and b are numbers */
	if (!value_is_number(a) || !value_is_number(b)) {
		return 1;
	}

	/*
	 * if both are integers, carry out operation as an integer operation
	 * otherwise carry out as reals;
	 */
	if (a->type == VALUE_TYPE_INTEGER && b->type == VALUE_TYPE_INTEGER) {
		return arith_integer(state, op, a, b, result);
	}

	/* make sure both values are reals */
	if (a->type == VALUE_TYPE_INTEGER) {
		integer_to_real(a);
	}
	if (b->type == VALUE_TYPE_INTEGER) {
		integer_to_real(b);
	}

	return arith_real(state, op, a, b, result);
}

/* carries out an arithmetic operation */
static int arith(struct state *state, enum vm_op op, struct value *a, struct value *b, struct value *result) {
	/*
	 * if a & b is both of the same type and is either real or integer,
	 * carry out the operation directly
	 */
	if (a->type == VALUE_TYPE_REAL && b->type == VALUE_TYPE_REAL) {
		return arith_real(state, op, a, b, result);	
	} else if (a->type == VALUE_TYPE_INTEGER && b->type == VALUE_TYPE_INTEGER) {
		return arith_integer(state, op, a, b, result);
	}

	/*
	 * otherwise try to carry out the operations while coercing the 
	 * values of a and b to a type which supports arithmetic
	 * operations (integer or real)
	 */
	return arith_coerce(state, op, a, b, result);
}

int vm_execute(struct state *state, struct func *fn) {
	/* TODO: optionally implement direct threading dispatching */

	/* check if we've got enough parameters on the stack */
	size_t space = (size_t)(state->sp - state->stack);
	if (fn->proto->nparams > space) {
		return 1;
	}

	/*
	 * ensure the stack for all the locals in the function
	 * as well as for the worst case senario, we push all
	 * constants on the stack
	 */
	if (!stack_ensure(state, fn->nlocals + fn->nconsts)) {
		return 1;
	}

	struct value *stack_base = state->sp;

	/* reset the instruction pointer */
	state->ip = fn->code;
	state->sp = stack_base + fn->nlocals;
	memset(state->stack, 0, sizeof(struct value) * fn->nlocals);

	for (uint8_t i = 0; i < fn->nlocals; i++) {
		struct value *val = stack_base + i;
		struct variable *var = &fn->locals[i];

		if (var->type == state->boolean_type) {
			val->type = VALUE_TYPE_BOOLEAN;
		} else if (var->type == state->integer_type) {
			val->type = VALUE_TYPE_INTEGER;
		} else if (var->type == state->real_type) {
			val->type = VALUE_TYPE_REAL;
		} else {
			val->type = VALUE_TYPE_OBJECT;
		}
	}

	/* vm dispatch loop */
	while (true) {
		/* fetch instruction at pc */
		enum vm_op op = (enum vm_op)*state->ip++;
		switch (op) {
			case VM_OP_HALT: {
				/* graceful exit */
				return 0;
			}
			case VM_OP_PUSH: {
				/*
				 * pushses the constant at `index` in 
				 * the current `fn` on the stack
				 */
				unsigned int index = (unsigned int)*state->ip++;
				struct value *val = &fn->consts[index];
				stack_push(state, val);
				break;
			}
			case VM_OP_GETLOCAL: {
				uint8_t index = *state->ip++;
				//struct variable *var = &fn->locals[index];
				struct value *val = stack_base + index;
				stack_push(state, val);	
				break;
			}
			case VM_OP_ADD:
			case VM_OP_SUB: 
			case VM_OP_MUL:
			case VM_OP_DIV: {
				/*
				 * pops the last 2 values from the stack
				 * and carry out the operation on them, 
				 * then push the result back on the stack
				 */
				struct value *a = stack_pop(state);
				struct value *b = stack_pop(state);	
				struct value result;

				/* carry out the arithmetic operation */
				if (arith(state, op, a, b, &result)) {
					return 1;
				}

				stack_push(state, &result);
				break;
			}
			case VM_OP_OUTPUT: {
				/*
				 * pop the top value on the stack and 
				 * print it.
				 */
				struct value *val = stack_pop(state);
				output(val);
				break;
			}
			default: {
				/* unknown/unhandled instruction */
				return 1;
			}
		}
	}
	return 0;
}
