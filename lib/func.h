#ifndef FUNCTION_H
#define FUNCTION_H

#include "opcode.h"
#include "state.h"

/*
 * represents a function
 */
struct func {
	struct proto *proto; /* prototype of the function */
	size_t constkc; /* constant count */
	struct value *constk; /* array of constants */
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
	size_t paramc; /* parameter count */
	struct type *paramt; /* array of parameter types */
	struct type *rett; /* return type */
};

void func_init(struct func *fn);
void func_deinit(struct func *fn);

#endif /* FUNCTION_H */
