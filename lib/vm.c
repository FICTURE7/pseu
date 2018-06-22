#include <stdio.h>
#include <stdbool.h>
#include "vm.h"
#include "state.h"
#include "object.h"
#include "vector.h"
#include "opcodes.h"
#include "function.h"

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

void vm_init(struct vm *vm, struct state *state) {
	vm->state = state;
	vm->pc = 0;
	vm->sp = 0;
}

int vm_exec(struct vm *vm, struct function *fn) {
	while (true) {
		/* fetch instruction at pc */
		enum vm_op op = (enum vm_op)fn->code[vm->pc++];
		switch (op) {
			case VM_OP_HALT: {
				return 0;
			}
			case VM_OP_PUSH_OBJECT: {
				unsigned int index = fn->code[vm->pc++];
				struct value val = {
					.type = VALUE_TYPE_OBJECT,
					.as_object = (struct object *)vector_get(&fn->consts, index)
				};
				stack_push(vm, &val);
				break;
			}
			case VM_OP_OUTPUT: {
				struct value val = stack_pop(vm);
				struct string_object *str = (struct string_object *)val.as_object;
				printf("%s\n", str->buf);
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
