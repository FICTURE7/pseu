#ifndef VM_H
#define VM_H

#include "state.h"
#include "object.h"
#include "function.h"

/*
 * virtual machine implementaiton
 */
struct vm {
	/* program counter */
	int pc;
	/* stack pointer */
	int sp;
	/* stack */
	struct value stack[256];
};

void vm_init(struct vm *vm);

int vm_exec(struct vm *vm, struct state *state, struct function *fn);

#endif /* VM_H */
