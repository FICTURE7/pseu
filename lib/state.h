#ifndef STATE_H
#define STATE_H

#include <pseu.h>
#include "func.h"
#include "value.h"
#include "string.h"
#include "opcode.h"

/*
 * represents a state
 */
struct state {
	struct diagnostic *errors; /* linked list of errors of this state instance */
	struct string_table *strings; /* string table in the state */
	struct value *sp;	/* stack pointer (points to top of stack) */
	instr_t *ip; /* instruction pointer */
	/* TODO: implement dynamic stack */
	struct value stack[256]; /* stack */
};

void state_init(struct state *state);
void state_deinit(struct state *state);

#endif /* STATE_H */
