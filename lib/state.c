#include <pseu.h>
#include <stdlib.h>
#include "state.h"
#include "string.h"
#include "diagnostic.h"

void state_init(struct state *state, pseu_config_t *config) {
	state->config = config;

	state->stack = malloc(sizeof(struct value) * config->init_stack_size);
	state->stack_top = state->stack + config->init_stack_size;
	state->sp = state->stack;

	state->ip = NULL;
	state->errors = NULL;

	state->strings = malloc(sizeof(struct string_table));	
	/* intialize the state' string table */
	string_table_init(state->strings);
}

void state_deinit(struct state *state) {
	/* free the linked list of diagnostics */
	struct diagnostic *current;
	struct diagnostic *next;

	current = state->errors;
	for (; current != NULL; current = next) {
		next = current->next;
		free(current);
	}

	string_table_deinit(state->strings);
	//free(state->strings);
	free(state->stack);
}
