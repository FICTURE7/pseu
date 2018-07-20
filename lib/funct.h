#ifndef FUNCTION_H
#define FUNCTION_H

#include "vector.h"
#include "opcode.h"

/*
 * represents a function
 */
struct funct {
	/* list of constants in the function */
	struct vector consts;
	/* instructions in the function */
	vm_instr_t *code;
};

void funct_init(struct funct *fn);
void funct_deinit(struct funct *fn);

#endif /* FUNCTION_H */
