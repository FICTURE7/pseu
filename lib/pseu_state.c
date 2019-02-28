#include <pseu.h>
#include <assert.h>
#include <stdlib.h>

#include "pseu_vm.h"
#include "pseu_state.h"
#include "pseu_debug.h"

/* Initial evaluation stack size. */
#define INIT_EVALSTACK_SIZE 256
/* Initial call stack size. */
#define INIT_CALLSTACK_SIZE 8

void state_init(pseu_vm_t *vm, struct state *state) {
	pseu_assert(state && vm);

	state->vm = vm;

	/* Initialize the state's evaluation stack. */
	state->stack_size = INIT_EVALSTACK_SIZE;
	state->stack = pseu_alloc(vm, state->stack_size * sizeof(struct value));
	state->sp = state->stack;

	/* Initialize the state's call frame stack. */
	state->frames_count = 0;
	state->frames_size = INIT_CALLSTACK_SIZE;
	state->frames = pseu_alloc(vm, state->frames_size * sizeof(struct frame));

	state->error = NULL;
}

void state_deinit(struct state *state) {
	pseu_assert(state);

	/* Free stacks. */
	pseu_free(state->vm, state->stack);
	pseu_free(state->vm, state->frames);

	/* Free error, if set. */
	if (state->error) {
		pseu_free(state->vm, state->error);
	}
}
