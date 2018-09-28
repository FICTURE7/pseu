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
	 * instruction is going to write
	 * to
	 */
	struct value *sp; 

	/*
	 * base pointer
	 *
	 * points to the base of current frame
	 * in the stack
	 */
	struct value *bp;

	/*
	 * stack
	 *
	 * points to the bottom of the stack
	 */
	struct value *stack; 
	size_t cstack; 

	/* call frames */
	struct frame *frames; 
	size_t cframes;
	size_t nframes; 

	/*
	 * current error of the state
	 * NULL when no error
	 */
	struct error *error;

	/*
	 * pointer to  the virtual machine
	 * which owns this state instance
	 */
	pseu_vm_t *vm; 
};

/*
 * represents a call frame
 */
struct frame {
	struct func *fn; /* function of called in the frame */

	code_t *ip;
	struct value *base; /* stack base pointer of the frame */
	//struct value *bp; /* base pointer */
};

void state_init(struct state *state, struct vm *vm);
void state_deinit(struct state *state);

#endif /* STATE_H */
