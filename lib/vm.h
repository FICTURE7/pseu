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

enum vm_result vm_execute(struct state *state, struct func *fn);

#endif /* VM_H */
