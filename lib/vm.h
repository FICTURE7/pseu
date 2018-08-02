#ifndef VM_H
#define VM_H

#include "node.h"
#include "func.h"
#include "state.h"
#include "value.h"

/*
 * represents the results of
 * the execution of a function
 * by the vm
 */
enum vm_result {
	VM_RESULT_SUCCESS,
	VM_RESULT_ERROR
};

instr_t *vm_gen(struct node *node);
enum vm_result vm_call(struct state *state, struct func *fn);
enum vm_result vm_exec(struct state *state, instr_t *code);

#endif /* VM_H */
