#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "ssvm.h"
#include "../vm.h"

struct ssvm_data {
	int pc;
	int sp;
	int stack[256];
};

/* fetches the next instruction to be executed */
static int fetch(struct vm *vm, struct ssvm_ir *ir) {
	struct ssvm_data *data = vm->data;
	return (int)vector_get(&ir->instructions, data->pc);
}

/* push a 32-bit integer on the stack */
static void push(struct ssvm_data *ssvm, int val) {
	ssvm->stack[ssvm->sp++] = val;
}

/* pops a 32-bit integer from the stack */
static int pop(struct ssvm_data *ssvm) {
	return ssvm->stack[--ssvm->sp];
}

static void init(struct vm *vm) {
	struct ssvm_data *data = malloc(sizeof(struct ssvm_data));
	data->pc = 0;
	data->sp = 0;
	vm->data = data;
}

static void deinit(struct vm *vm) {
	free(vm->data);
}

static void eval(struct vm *vm, struct ssvm_ir *ir) {
	bool running = true;
	struct ssvm_data *ssvm = vm->data;

	while (running) {
		enum ssvm_ir_inst inst = fetch(vm, ir);
		switch (inst) {
			case SSVM_INST_RET: {
				running = false;
				break;
			}
			case SSVM_INST_PUSH: {
				ssvm->pc++;
				int val = fetch(vm, ir);
				push(ssvm, val);
				break;
			}
			case SSVM_INST_POP: {
				int val = pop(ssvm);
				printf("%d\n", val);
				break;
			}
			case SSVM_INST_ADD:
			case SSVM_INST_SUB:
			case SSVM_INST_MUL:
			case SSVM_INST_DIV: {
				int val1 = pop(ssvm);
				int val2 = pop(ssvm);
				int result;
				switch (inst) {
					case SSVM_INST_ADD:
						result = val1 + val2;
						break;
					case SSVM_INST_SUB:
						result = val1 - val2;
						break;
					case SSVM_INST_MUL:
						result = val1 * val2;
						break;
					case SSVM_INST_DIV:
						result = val1 / val2;
						break;
				}
				push(ssvm, result);
				break;
			}
		}
		ssvm->pc++;
	}
}

void vm_ssvm_init(struct vm *vm) {
	vm->init = init;
	vm->deinit = deinit;
	vm->eval = (void (*)(struct vm *, void *))eval;
}
