#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>

/* 
 * X Macro of vm operations
 * check https://en.wikipedia.org/wiki/X_Macro
 * for more information
 */
#define VM_OP 		\
	OP(RET) 		\
	OP(ADD) 		\
	OP(SUB) 		\
	OP(MUL) 		\
	OP(DIV) 		\
	OP(PUSH) 		\
	OP(POP) 		\
	OP(LD_LOCAL) 	\
	OP(ST_LOCAL)	\
	OP(OUTPUT) 		\
	OP(CALL)		\

/*
 * represents a code
 * which the virtual machine 
 * can execute
 */
enum code {
	#define OP(x) VM_OP_##x,
	VM_OP
	#undef OP
};

typedef enum code code_t;

#endif /* OPCODE_H */
