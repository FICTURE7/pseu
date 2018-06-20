#include "state.h"
#include "vm.h"

void vm_init(struct vm *vm) {
	vm->pc = 0;
	vm->sp = 0;
}

int vm_exec(struct vm *vm, struct state *state, vm_inst_t *inst) {
	return 0;
}
