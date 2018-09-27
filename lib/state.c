#include <pseu.h>
#include <stdlib.h>
#include "vm.h"
#include "state.h"
#include "value.h"
#include "string.h"
#include "diagnostic.h"

void state_init(struct state *state, pseu_vm_t *vm) {
	state->vm = vm;
	state->frames = malloc(sizeof(struct frame) * 8);
	
	state->stack = malloc(sizeof(struct value) * vm->config.init_stack_size);
	state->stack_top = state->stack + vm->config.init_stack_size;
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
