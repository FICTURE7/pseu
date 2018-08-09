#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdlib.h>
#include "value.h"
#include "opcode.h"
#include "location.h"

/*
 * represents a function
 */
struct func {
	struct proto *proto; /* prototype of the function */
	size_t nconsts; /* constant count */
	struct value *consts; /* array of constants */
	size_t ncode; /* number of instructions in the function */
	instr_t *code; /* instructions in the function */
};

/*
 * represents a function prototype
 */
struct proto {
	char *ident; /* identifier of the function */
	size_t nparams; /* parameter count or arity */
	struct type *params; /* array of parameter types */
	struct type *rett; /* return type */
	struct location location; /* where in the source code the proto was definied */
};

/*
 * represents a call
 */
struct call {
	struct proto *proto; /* prototype of the function */
};

void func_init(struct func *fn);
void func_deinit(struct func *fn);

#endif /* FUNCTION_H */
