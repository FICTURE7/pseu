#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>

/* represents an instruction */
typedef int8_t instr_t;

/*
 * operations the virtual machine is capabale of executing
 */
enum vm_op {
	VM_OP_HALT,

	VM_OP_ADD,
	VM_OP_SUB,
	VM_OP_MUL,
	VM_OP_DIV,

	VM_OP_PUSH,
	VM_OP_POP,
	
	VM_OP_OUTPUT
};

#endif /* OPCODE_H */
