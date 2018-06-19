/*
 * SSVM - Super Simple Virtual Machine
 * 
 *     Stack based virtual machine that
 * is(should be) simplistic.
 */

#ifndef SSVM_H
#define SSVM_H

#include <stdlib.h>
#include "../vector.h"
#include "../node.h"
#include "../vm.h"

enum ssvm_ir_inst {
	SSVM_INST_RET,

	SSVM_INST_OUTPUT,

	SSVM_INST_PUSH,
	SSVM_INST_POP,

	SSVM_INST_ADD,
	SSVM_INST_SUB,
	SSVM_INST_MUL,
	SSVM_INST_DIV
};

struct ssvm_ir {
	struct vector instructions;
};

struct ssvm_ir *vm_ssvm_ir_gen(struct vm *vm, struct node *node);

void vm_ssvm_init(struct vm *vm);

#endif /* SSVM_H */
