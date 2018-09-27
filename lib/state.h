#ifndef STATE_H
#define STATE_H

#include <pseu.h>
#include "error.h"
#include "value.h"
#include "opcode.h"

/*
 * represents the global state
 * of a pseu instance
 */
struct state {
	/*
	 * current error of the state
	 * NULL when no error
	 */
	struct error *error;

	/* 
	 * instruction pointer
	 *
	 * address of the instruction
	 * we're executing
	 */
	code_t *ip; 

	/* 
	 * stack pointer
	 *
	 * address at which the next PUSH
	 * instruction if going to write
	 * to
	 */
	struct value *sp; 

	struct value *stack; /* points to the bottom of the stack */
	size_t cstack; /* size of the stack */

	struct frame *frames; /* call frames */
	size_t nframes; /* number of call frames */

	pseu_vm_t *vm; /* vm which owns this state */
};

/*
 * represents a call frame
 */
struct frame {
	struct func *fn; /* function of called in the frame */
	struct value *base; /* stack base pointer of the frame */
};

void state_init(struct state *state, pseu_vm_t *vm);
void state_deinit(struct state *state);

#endif /* STATE_H */
