#ifndef STATE_H
#define STATE_H

#include "string.h"
#include "vector.h"

/* represents a state */
struct state {
	struct vector *diagnostics; /* list of diagnostics related to the state. */
	struct string_table *strings; /* string table in the state. */

	void (*onerror)(int errcode); /* function to call whenever stuff happens. */
};

void state_init(struct state *state);
void state_deinit(struct state *state);

#endif /* STATE_H */
