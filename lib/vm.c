#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "vm.h"
#include "func.h"
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
 * pops a value from the top of the
 * stack of the virtual machine
 * 
 * TODO: check underflow
 */
static inline struct value *stack_pop(struct vm *vm) {
	return &vm->stack[--vm->sp];
}

/*
 * pushes a value on top of the
 * stack of the virtual machine
 *
 * TODO: check overflow
 */
static inline void stack_push(struct vm *vm, struct value *val) {
	vm->stack[vm->sp++] = *val;
}

/* passes the control to the error handler */
static inline void error(struct vm *vm, struct diagnostic *err) {
	vm->error = err;
	vm->onerror(err);
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
		case VALUE_TYPE_OBJECT:
			/* TODO: handle arrays as well */
			if (value->as_object->type == &string_type) {
				struct string_object *string = (struct string_object *)value->as_object;
				printf("%s\n", string->buf);
			} else {
				printf("object<%p>\n", value->as_object);
			}
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
	lexer_init(&lexer, NULL, str->buf);
	lexer_scan(&lexer, &token);

	/* convert to the appropriate type */
	switch (token.type) {
		case TOK_LIT_INTEGER:
			result->type = VALUE_TYPE_INTEGER;
			result->as_int = strtol(str->buf, NULL, 10);
			break;
		case TOK_LIT_INTEGERHEX:
			result->type = VALUE_TYPE_INTEGER;
			result->as_int = strtol(str->buf, NULL, 16);
			break;
		case TOK_LIT_REAL:
			result->type = VALUE_TYPE_REAL;
			result->as_float = strtof(str->buf, NULL);
			break;

		default:
			/* non number literal token */
			return 1;
	}

	/*
	 * expect the next token to be EOF, which means
	 * we strict on the coercions
	 *
	 * so things like this "12 oixD" is not coerced
	 * to 12
	 */
	lexer_scan(&lexer, &token);
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
static int arith_real(struct vm *vm, enum vm_op op, struct value *a, struct value *b, struct value *result) {
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
				error(vm, NULL);
				return 1;
			}

			BINOP_REAL(/, a, b, result);
			return 0;

		default:
			return 1;
	}
}

/* carries out arithmetic operations on integer values */
static int arith_integer(struct vm *vm, enum vm_op op, struct value *a, struct value *b, struct value *result) {	
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
static int arith_coerce(struct vm *vm, enum vm_op op, struct value *a, struct value *b, struct value *result) {
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
	
	/* check if both a and b is a number */
	if (!value_is_number(a) || !value_is_number(b)) {
		return 1;
	}

	/*
	 * if both is integer, carry out operation as integer
	 * otherwise carry out as floats
	 */
	if (a->type == VALUE_TYPE_INTEGER && b->type == VALUE_TYPE_INTEGER) {
		return arith_integer(vm, op, a, b, result);
	}

	/* make sure both values are reals */
	if (a->type == VALUE_TYPE_INTEGER) {
		integer_to_real(a);
	}
	if (b->type == VALUE_TYPE_INTEGER) {
		integer_to_real(b);
	}

	return arith_real(vm, op, a, b, result);
}

/* carries out an arithmetic operation */
static int arith(struct vm *vm, enum vm_op op, struct value *a, struct value *b, struct value *result) {
	/*
	 * if a & b is both of the same type and is either real or integer,
	 * carry out the operation directly
	 */
	if (a->type == VALUE_TYPE_REAL && b->type == VALUE_TYPE_REAL) {
		return arith_real(vm, op, a, b, result);	
	} else if (a->type == VALUE_TYPE_INTEGER && b->type == VALUE_TYPE_INTEGER) {
		return arith_integer(vm, op, a, b, result);
	}

	/*
	 * otherwise try to carry out the operations while coercing the 
	 * values of a and b to type which supports arithmetic
	 * operations (integer or real)
	 */
	return arith_coerce(vm, op, a, b, result);
}

void vm_init(struct vm *vm, struct state *state) {
	vm->state = state;
	vm->pc = 0;
	vm->sp = 0;
}

int vm_exec(struct vm *vm, struct func *fn) {
	/* set the current vm call to the fn passed */
	struct call *call = malloc(sizeof(struct call));
	call->proto = fn->proto;
	vm->call = call;

	/* vm dispatch loop */
	while (true) {
		/* fetch instruction at pc */
		enum vm_op op = (enum vm_op)fn->code[vm->pc++];
		switch (op) {
			case VM_OP_HALT: {
				/* graceful exit */
				return VM_RESULT_SUCCESS;
			}
			case VM_OP_PUSH: {
				/*
				 * pushses the constant at `index` in 
				 * the current `fn` on the stack
				 */
				unsigned int index = (int)fn->code[vm->pc++];
				struct value *val = &fn->consts[index];
				stack_push(vm, val);
				break;
			}
			case VM_OP_ADD: {
				/*
				 * pops the last 2 values fromt he stack
				 * and adds them, then push the result
				 * back on the stack
				 */
				struct value *a = stack_pop(vm);
				struct value *b = stack_pop(vm);	
				struct value result;

				arith(vm, VM_OP_ADD, a, b, &result);

				stack_push(vm, &result);
				break;
			}
			case VM_OP_OUTPUT: {
				/*
				 * pop the top value on the stack and 
				 * print it.
				 */
				struct value *val = stack_pop(vm);
				output(val);
				break;
			}
			default: {
				/* unknown/unhandled instruction */
				vm->onerror(1);
				return 1;
			}
		}
	}
	return 0;
}
