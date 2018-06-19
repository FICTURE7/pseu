#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "vm.h"
#include "object.h"

/* fetches the next instruction to be executed */
static int fetch(struct vm *vm, struct vm_ir *ir) {
	return (int)vector_get(&ir->instructions, vm->pc);
}

/* push a 32-bit integer on the stack */
static void push(struct vm *vm, struct value *val) {
	vm->stack[vm->sp++] = val;
}

/* pops a 32-bit integer from the stack */
static struct value *pop(struct vm *vm) {
	return (struct value *)vm->stack[--vm->sp];
}

void vm_eval(struct vm *vm, struct vm_ir *ir) {
	bool running = true;

	while (running) {
		enum vm_ir_inst inst = fetch(vm, ir);
		switch (inst) {
			case VM_INST_RET: {
				running = false;
				break;
			}
			case VM_INST_OUTPUT: {
				struct value *val = pop(vm);
				break;
			}
			case VM_INST_PUSH: {
				vm->pc++;
				int val = fetch(vm, ir);
				push(vm, val);
				break;
			}
			case VM_INST_POP: {
				int val = pop(vm);
				break;
			}
			case VM_INST_ADD:
			case VM_INST_SUB:
			case VM_INST_MUL:
			case VM_INST_DIV: {
				int val1 = pop(vm);
				int val2 = pop(vm);
				int result;
				switch (inst) {
					case VM_INST_ADD:
						result = val1 + val2;
						break;
					case VM_INST_SUB:
						result = val1 - val2;
						break;
					case VM_INST_MUL:
						result = val1 * val2;
						break;
					case VM_INST_DIV:
						result = val1 / val2;
						break;
				}
				push(vm, result);
				break;
			}
		}
		vm->pc++;
	}
}

void vm_init(struct vm *vm) {
	vm->pc = 0;
	vm->sp = 0;
}
