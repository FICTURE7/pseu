#ifndef FUNCTION_H
#define FUNCTION_H

#include "opcode.h"
#include "state.h"

/*
 * represents a function
 */
struct func {
	struct proto *proto; /* prototype of the function */
	size_t nconsts; /* constant count */
	struct value *consts; /* array of constants */
	instr_t *code; /* instructions in the function */
};


/*
 * represents a C function
 */
struct cfunc {
	struct proto *proto; /* prototype of the function */
	int (*fn)(struct state); /* C function pointer */
};

/*
 * represents a function prototype
 */
struct proto {
	char *ident; /* identifier of the function */
	size_t nparams; /* parameter count */
	struct type *paramst; /* array of parameter types */
	struct type *rett; /* return type */
};

void func_init(struct func *fn);
void func_deinit(struct func *fn);

#endif /* FUNCTION_H */
