#include <stdlib.h>
#include "state.h"
#include "string.h"
#include "diagnostic.h"

void state_init(struct state *state) {
	state->strings = malloc(sizeof(struct string_table));
	state->ip = NULL;
	state->sp = state->stack;
	
	/* intialize the state' string table */
	string_table_init(state->strings);
}

void state_deinit(struct state *state) {
	/* free the linked list of chains */
	struct diagnostic *current;
	struct diagnostic *next;

	current = state->errors;
	for (; current != NULL; current = next) {
		next = current->next;
		free(current);
	}

	string_table_deinit(state->strings);
	free(state->strings);
}
