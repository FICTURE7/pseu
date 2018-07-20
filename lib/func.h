#ifndef FUNCTION_H
#define FUNCTION_H

#include "vector.h"
#include "opcode.h"

/*
 * represents a function
 */
struct func {
	/* list of constants in the function */
	struct vector consts;
	/* instructions in the function */
	vm_instr_t *code;
};

void func_init(struct func *fn);
void func_deinit(struct func *fn);

#endif /* FUNCTION_H */
