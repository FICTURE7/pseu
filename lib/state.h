#ifndef STATE_H
#define STATE_H

#include "string.h"
#include "vector.h"

struct state {
	struct vector *diagnostics;
	struct string_table *strings;
};

void state_init(struct state *state);

void state_deinit(struct state *state);

#endif /* STATE_H */
