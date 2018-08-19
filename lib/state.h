#ifndef STATE_H
#define STATE_H

#include <pseu.h>
#include "func.h"
#include "value.h"
#include "string.h"
#include "opcode.h"
#include "symbol.h"

/*
 * represents the global state
 * of a pseu instance
 */
struct state {
	struct diagnostic *errors; /* linked list of errors of this state instance */
	struct string_table *strings; /* string table for interning strings */
	struct symbol_table *symbols; /* symbols in the state */

	instr_t *ip; /* instruction pointer */
	struct value *sp;	/* stack pointer (points to top of stack) */
	struct value *stack; /* points to the bottom of the stack */
	struct value *stack_top; /* points to the top of the stack */

	/* primitive built-in types */
	struct type *void_type;
	struct type *string_type;
	struct type *array_type;

	pseu_config_t *config; /* config of the state */
};

void state_init(struct state *state, pseu_config_t *config);
void state_deinit(struct state *state);

#endif /* STATE_H */
