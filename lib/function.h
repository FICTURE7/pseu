#ifndef FUNCTION_H
#define FUNCTION_H

#include "vector.h"
#include "opcodes.h"

/*
 * represents a function
 */
struct function {
	/* list of constants in this function */
	struct vector consts;
	vm_instr_t *code;
};

void function_init(struct function *fn);

#endif /* FUNCTION_H */
