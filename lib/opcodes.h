#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>

typedef int32_t vm_instr_t;

/*
 * operations the virtual machine is capabale of executing
 */
enum vm_op {
	VM_OP_HALT,

	VM_OP_PUSH_BOOLEAN,
	VM_OP_PUSH_REAL,
	VM_OP_PUSH_INTEGER,
	VM_OP_PUSH_OBJECT,
	VM_OP_POP,
	
	VM_OP_OUTPUT
};

#endif /* OPCODES_H */
