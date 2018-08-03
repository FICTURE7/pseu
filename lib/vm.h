#ifndef VM_H
#define VM_H

#include "func.h"
#include "node.h"
#include "state.h"

/*
 * represents the results of
 * the execution of a function
 * by the vm
 */
enum vm_result {
	VM_RESULT_SUCCESS,
	VM_RESULT_ERROR
};

struct func *vm_gen(struct node *node);
enum vm_result vm_call(struct state *state, struct func *fn);

#endif /* VM_H */
