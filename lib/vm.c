#include <stdio.h>
#include <stdbool.h>
#include "vm.h"
#include "func.h"
#include "state.h"
#include "value.h"
#include "opcode.h"

/* 
 * carries out the specified operation on
 * the specified operand _a and _b which are in
 * the type of `struct value`, as floats
 */
#define BINOP_REAL(_op, _a, _b) 							\
		(struct value) { 									\
			.type = VALUE_TYPE_REAL, 						\
			.as_float = (_a)->as_float _op (_b)->as_float 	\
		}

/* 
 * carries out the specified operation on
 * the specified operand _a and _b which are in
 * the type of `struct value`, as ints
 */
#define BINOP_INTEGER(_op, _a, _b) 							\
		(struct value) { 									\
			.type = VALUE_TYPE_INTEGER,						\
			.as_int = (_a)->as_int _op (_b)->as_int 		\
		}

/*
 * pops a value from the top of the
 * stack of the virtual machine
 * 
 * TODO: check underflow
 */
static struct value stack_pop(struct vm *vm) {
	return vm->stack[--vm->sp];
}

/*
 * pushes a value on top of the
 * stack of the virtual machine
 *
 * TODO: check overflow
 */
static void stack_push(struct vm *vm, struct value *val) {
	vm->stack[vm->sp++] = *val;
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

static struct value string_to_number(struct value *a) {
	return (struct value) { };
}

/* carries out arithmetic operations on real values */
static struct value arith_real(enum vm_op op, struct value *a, struct value *b) {
	switch (op) {
		case VM_OP_ADD:
			return BINOP_REAL(+, a, b);
		case VM_OP_SUB:
			return BINOP_REAL(-, a, b);
		case VM_OP_MUL:
			return BINOP_REAL(*, a, b);
		case VM_OP_DIV:
			return BINOP_REAL(/, a, b);
		default:
			/* TODO: push error */
			break;
	}
	return (struct value) { };
}

/* carries out arithmetic operations on integer values */
static struct value arith_integer(enum vm_op op, struct value *a, struct value *b) {	
	switch (op) {
		case VM_OP_ADD:
			return BINOP_INTEGER(+, a, b);
		case VM_OP_SUB:
			return BINOP_INTEGER(-, a, b);
		case VM_OP_MUL:
			return BINOP_INTEGER(*, a, b);
		case VM_OP_DIV:
			return BINOP_INTEGER(/, a, b);
		default:
			/* TODO: push error */
			break;
	}
	return (struct value) { };
}

/* coerce values and carries out arithemtic operations on them */
static struct value arith_coerce(enum vm_op op, struct value *a, struct value *b) {
	/* coerce stuff */
	if (value_is_string(a)) {
		a = &(string_to_number(a));
	}
	if (value_is_string(b)) {
		b = &(string_to_number(b));
	}

	/*
	 * if both is integer, carry out operation as integer
	 * otherwise carry out as floats
	 */
	if (a->type == VALUE_TYPE_INTEGER && b->type == VALUE_TYPE_INTEGER) {
		return arith_integer(op, a, b);
	}

	return arith_real(op, a, b);	
}

/* carries out an arithmetic operation */
static struct value arith(enum vm_op op, struct value *a, struct value *b) {
	if (a->type == VALUE_TYPE_REAL && b->type == VALUE_TYPE_REAL) {
		return arith_real(op, a, b);	
	} else if (a->type == VALUE_TYPE_INTEGER && b->type == VALUE_TYPE_INTEGER) {
		return arith_integer(op, a, b);
	}

	/* try to carry out operation with coercion */
	return arith_coerce(op, a, b);	
}

void vm_init(struct vm *vm, struct state *state) {
	vm->state = state;
	vm->pc = 0;
	vm->sp = 0;
}

int vm_exec(struct vm *vm, struct func *fn) {
	/* vm dispatch loop */
	while (true) {
		/* fetch instruction at pc */
		enum vm_op op = (enum vm_op)fn->code[vm->pc++];
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
				unsigned int index = fn->code[vm->pc++];
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
				struct value a = stack_pop(vm);
				struct value b = stack_pop(vm);	
				struct value result = arith(VM_OP_ADD, &a, &b);
				stack_push(vm, &result);
				break;
			}
			case VM_OP_OUTPUT: {
				/*
				 * pop the top value on the stack and 
				 * print it.
				 */
				struct value val = stack_pop(vm);
				output(&val);
				break;
			}
			default: {
				/* unknown/unhandled instruction */
				vm->state->onerror(1);
				return 1;
			}
		}
	}
	return 0;
}
