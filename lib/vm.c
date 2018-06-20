#include "vm.h"

void vm_init(struct vm *vm) {
	vm->pc = 0;
	vm->sp = 0;
}

void vm_exec(struct vm *vm, struct vm_ir *ir) {

}
