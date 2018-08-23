#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>

/* represents an instruction */
typedef uint8_t instr_t;

/* 
 * X Macro of vm operations
 * check https://en.wikipedia.org/wiki/X_Macro
 * for more information
 */
#define VM_OP 		\
	OP(HALT) 		\
	OP(ADD) 		\
	OP(SUB) 		\
	OP(MUL) 		\
	OP(DIV) 		\
	OP(PUSH) 		\
	OP(POP) 		\
	OP(GETLOCAL)	\
	OP(GETGLOBAL)	\
	OP(SETLOCAL) 	\
	OP(SETGLOBAL)	\
	OP(OUTPUT) 

/*
 * operations the virtual machine is capabale of executing
 */
enum vm_op {
	#define OP(_name) VM_OP_##_name,
	VM_OP
	#undef OP
};

#endif /* OPCODE_H */
