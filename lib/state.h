#ifndef STATE_H
#define STATE_H

#include "func.h"
#include "string.h"
#include "vector.h"

/*
 * represents a state
 */
struct state {
	struct diagnostic *diagnostics; /* linked list of diagnostics related to the state */
	struct string_table *strings; /* string table in the state */
	struct func *main; /* main function */
};

void state_init(struct state *state);
void state_deinit(struct state *state);

#endif /* STATE_H */
