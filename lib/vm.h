#ifndef VM_H
#define VM_H

#include "state.h"
#include "value.h"
#include "funct.h"

/*
 * virtual machine implementaiton
 */
struct vm {
	/* state which owns the vm instance */
	struct state *state;
	/* program counter */
	int pc;
	/* stack pointer */
	int sp;
	/* 
	 * stack
	 * TODO: implement dynamic stack
	 */
	struct value stack[256 * sizeof(struct value)];
};

void vm_init(struct vm *vm, struct state *state);
int vm_exec(struct vm *vm, struct funct *fn);

#endif /* VM_H */
