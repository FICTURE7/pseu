#include <pseu.h>
#include <stdio.h>
#include <stdlib.h>

#include "vm.h"
#include "state.h"
#include "value.h"
#include "opcode.h"
#include "symbol.h"

int main(int argc, char **argv) {
	struct vm *vm = pseu_vm_new(NULL);
	struct state *state = &vm->state;

	struct func add_fn = {
		.proto = &(struct proto) {
			.ident = "add",
			.stack_size = 2,
			.nparams = 2,
			.param_types = (struct type*[]) {
				&vm->integer_type,
				&vm->integer_type
			},
			.return_type = &vm->void_type
		},
		.code = (code_t[]) {
			VM_OP_LD_LOCAL, 1,
			VM_OP_LD_LOCAL, 0,
			VM_OP_ADD,
			VM_OP_RET
		}
	};

	symbol_table_add_func(vm->symbols, &add_fn);

	struct func main_fn = {
		.proto = &(struct proto) {
			.ident = NULL,
			.stack_size = 3,
		},
		.nconsts = 2,
		.consts = (struct value[]) {
			{ 
				.type = VALUE_TYPE_INTEGER,
				.as_int = 100
			},
			{ 
				.type = VALUE_TYPE_INTEGER,
				.as_int = 1001
			}
		},
		.code = (code_t[]) {
			VM_OP_PUSH, 0,
			VM_OP_PUSH, 1,
			VM_OP_CALL, 0,
			VM_OP_OUTPUT,
			VM_OP_RET
		}
	};

	int result = vm_call(state, &main_fn);
	printf("result: %d\n", result);

	pseu_free(vm);
	return result;
}
