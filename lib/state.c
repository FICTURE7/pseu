#include "state.h"
#include "string.h"
#include "vector.h"

void state_init(struct state *state) {
	state->diagnostics = malloc(sizeof(struct vector));
	state->strings = malloc(sizeof(struct string_table));

	vector_init(state->diagnostics);
	string_table_init(state->strings);
}

void state_deinit(struct state *state) {
	/* free diagnostics allocations */
	for (unsigned int i = 0; i < state->diagnostics->count; i++) {
		free(vector_get(state->diagnostics, i));
	}
	vector_deinit(state->diagnostics);
	string_table_deinit(state->strings);
}
