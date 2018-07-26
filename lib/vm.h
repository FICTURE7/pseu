#ifndef VM_H
#define VM_H

#include "state.h"
#include "value.h"
#include "func.h"

/*
 * virtual machine implementaiton
 */
struct vm {
	struct state *state; /* state which owns the vm instance */
	struct call *call; /* current call */
	int pc; /* program counter */
	int sp;	/* stack pointer */

	/* TODO: implement dynamic stack */
	struct value stack[256 * sizeof(struct value)]; /* stack */

	void (*onerror)(struct diagnostic *err); /* function to call whenever stuff happens */
};

void vm_init(struct vm *vm, struct state *state);
int vm_exec(struct vm *vm, struct func *fn);

#endif /* VM_H */
