#ifndef FUNCTION_H
#define FUNCTION_H

#include "vector.h"
#include "opcodes.h"

/*
 * represents a function
 */
struct function {
	/* list of constants in the function */
	struct vector consts;
	/* instructions in the function */
	vm_instr_t *code;
};

void function_init(struct function *fn);

void function_deinit(struct function *fn);

#endif /* FUNCTION_H */
