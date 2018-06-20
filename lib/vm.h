#ifndef VM_H
#define VM_H

#include "state.h"

/*
 * represents a virtual machine instruction
 */
typedef int vm_inst_t;

/*
 * operations the virtual machine is capabale of executing
 */
enum vm_inst_op {
	VM_INST_OP_HALT,

	VM_INST_OP_PUSH,
	VM_INST_OP_POP,
	
	VM_INST_OP_OUTPUT
};

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

int vm_exec(struct vm *vm, struct state *state, vm_inst_t *inst);

#endif /* VM_H */
