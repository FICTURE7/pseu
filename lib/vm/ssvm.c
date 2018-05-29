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

void init(struct vm *vm) {
	struct ssvm_data *data = malloc(sizeof(struct ssvm_data));
	data->pc = 0;
	data->sp = 0;
	vm->data = data;
}

void deinit(struct vm *vm) {
	free(vm->data);
}

void eval(struct vm *vm, void *p) {
	bool running = true;
	struct ssvm_ir *ir = p;
	struct ssvm_data *data = vm->data;

	while (running) {
		enum ssvm_ir_inst instr = (int)vector_get(&ir->instructions, data->pc);
		switch (instr) {
			case SSVM_INST_RET: {
				running = false;
				break;
			}
			case SSVM_INST_PUSH: {
				int val = (int*)vector_get(&ir->instructions, ++data->pc);
				data->stack[data->sp++] = val;
				break;
			}
			case SSVM_INST_POP: {
				int val = data->stack[--data->sp];
				printf("%d\n", val);
				break;
			}
			case SSVM_INST_ADD:
			case SSVM_INST_SUB:
			case SSVM_INST_MUL:
			case SSVM_INST_DIV: {
				int val1 = data->stack[--data->sp];
				int val2 = data->stack[--data->sp];
				int result;
				switch (instr) {
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
				data->stack[data->sp++] = result;
				break;
			}

			case SSVM_INST_TEST:
				printf("\nSSVM TEST\n");
				break;
		}
		data->pc++;
	}
}

void vm_ssvm_init(struct vm *vm) {
	vm->init = init;
	vm->deinit = deinit;
	vm->eval = eval;
}
