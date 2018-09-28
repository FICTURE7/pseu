#include <pseu.h>
#include <stdlib.h>
#include "vm.h"
#include "state.h"

void state_init(struct state *state, struct vm *vm) {
	state->vm = vm;
	
	state->cframes = 8;
	state->nframes = 0;
	state->frames = malloc(sizeof(struct frame) * state->cframes);

	state->cstack = vm->config.init_stack_size;
	state->stack = malloc(sizeof(struct value) * state->cstack);

	state->sp = state->stack;

	state->ip = NULL;
	state->error = NULL;
}

void state_deinit(struct state *state) {
	free(state->stack);
	free(state->frames);

	if (state->error != NULL) {
		free(state->error);
	}
}
