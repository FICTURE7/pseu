#ifndef VM_H
#define VM_H

#include "node.h"
#include "vector.h"

enum vm_ir_inst {
	VM_INST_RET,

	VM_INST_OUTPUT,

	VM_INST_PUSH,
	VM_INST_POP,

	VM_INST_ADD,
	VM_INST_SUB,
	VM_INST_MUL,
	VM_INST_DIV
};

/*
 * intermediate representation
 */
struct vm_ir {
	struct vector instructions;
};

/*
 * virtual machine
 * implementation
 */
struct vm {
	/* program counter */
	int pc;
	/* stack pointer */
	int sp;
	/* stack */
	void *stack[256];
};

struct vm_ir *vm_gen(struct vm *vm, struct node *node);

void vm_init(struct vm *vm);

void vm_exec(struct vm *vm, struct vm_ir *ir);

#endif /* VM_H */
