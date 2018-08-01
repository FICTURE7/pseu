#ifndef VM_H
#define VM_H

#include "func.h"
#include "state.h"
#include "value.h"

/*
 * virtual machine implementaiton
 */
struct vm {
	struct diagnostic *error; /* current error of the vm */
	struct pseu *pseu; /* pseu instance which owns this vm instance */
	struct state *state; /* state which owns the vm instance */
	struct call *call; /* current call */

	int pc; /* program counter */
	int sp;	/* stack pointer */
	/* TODO: implement dynamic stack */
	struct value stack[256]; /* stack */
};

/*
 * represents the results of
 * the execution of a function
 * by the vm
 */
enum vm_result {
	VM_RESULT_SUCCESS,
	VM_RESULT_ERROR
};

void vm_init(struct vm *vm, struct state *state);
enum vm_result vm_exec(struct vm *vm, struct func *fn);

#endif /* VM_H */
