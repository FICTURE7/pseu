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
	enum ssvm_instr *pos = ir->instr;

	while (running) {
		enum ssvm_instr instr = *pos;
		switch (instr) {
			case SSVM_INSTR_RET: {
				running = false;
				break;
			}
			case SSVM_INSTR_PUSH: {
				int val = *++pos;
				data->stack[data->sp++] = val;
				break;
			}
			case SSVM_INSTR_POP: {
				int val = data->stack[--data->sp];
				printf("%d\n", val);
				break;
			}
			case SSVM_INSTR_ADD: {
				int val1 = data->stack[--data->sp];
				int val2 = data->stack[--data->sp];
				int result = val1 + val2;
				data->stack[data->sp++] = result;
				break;
			}

			case SSVM_INSTR_TEST:
				printf("\nSSVM TEST\n");
				break;
		}
		pos++;
	}
}

void vm_ssvm_init(struct vm *vm) {
	vm->init = init;
	vm->deinit = deinit;
	vm->eval = eval;
}
