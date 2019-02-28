#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>

/* 
 * X Macro of virtual machine opcode.
 * Check https://en.wikipedia.org/wiki/X_Macro for more information.
 */
#define VM_OP \
	OP(END) \
	OP(RET) \
	OP(LD_CONST) \
	OP(LD_LOCAL) \
	OP(ST_LOCAL) \
	OP(LD_GLOBAL) \
	OP(ST_GLOBAL) \
	OP(CALL) \

/*
 * Opcodes which the pseu virtual machine supports.
 */
enum code {
	#define OP(x) VM_OP_##x,
	VM_OP
	#undef OP
};

/* Represents a pseu virutal machine instruction byte code. */
typedef uint8_t code_t;

#endif /* OPCODE_H */
