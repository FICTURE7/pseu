#ifndef PSEU_STATE_H
#define PSEU_STATE_H

#include <pseu.h>

#include "pseu_value.h"
#include "pseu_opcode.h"

/*
 * Represents a state of a pseu virtual machine.
 *
 * NOTE:
 * This is meant to be used per thread; but pseu does not support multi-threading.
 * So the default implementation uses a single global state instead.
 */
struct state {
	/* 
	 * Pointer to the virtual machine instance which owns this state instance. 
	 */
	pseu_vm_t *vm; 

	/* Stack pointer. */
	struct value *sp;
	/* Capacity of evaluation stack. */
	size_t stack_size;
	/* Evaluation stack of state. */
	struct value *stack;

	/* Number of frames in the call frame stack. */
	size_t frames_count;
	/* Capacity of call frame stack. */
	size_t frames_size;
	/* Call frame stack of state. */
	struct frame *frames;

	/* Error message. */
	char *error;
};

/*
 * Represents a call frame.
 */
struct frame {
	/* Pointer to closure of frame. */
	struct closure *closure;
	/* Base pointer of frame. */
	struct value *bp;
	/* Instruction pointer of frame. */
	code_t *ip;
};

/*
 * Initializes the specified state with the specified virtual machine instance.
 *
 * @param vm Virtual machine instance.
 * @param state State to initialize.
 */
void state_init(pseu_vm_t *vm, struct state *state);

/*
 * Cleans up the specified state.
 *
 * @param state State to clean up.
 */
void state_deinit(struct state *state);

#endif /* PSEU_STATE_H */
