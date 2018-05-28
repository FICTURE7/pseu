/*
 * SSVM - Super Simple Virtual Machine
 * 
 *     Stack based virtual machine that
 * is(should be) simplistic.
 */

#ifndef SSVM_H
#define SSVM_H

#include <stdlib.h>
#include "../vm.h"

enum ssvm_instr {
	SSVM_INSTR_RET,
	SSVM_INSTR_PUSH,
	SSVM_INSTR_POP,
	SSVM_INSTR_ADD,
	SSVM_INSTR_TEST
};

struct ssvm_ir {
	size_t len;
	enum ssvm_instr *instr;
};

void vm_ssvm_init(struct vm *vm);

#endif /* SSVM_H */
