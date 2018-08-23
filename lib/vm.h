#ifndef VM_H
#define VM_H

#include "node.h"
#include "state.h"
#include "value.h"

int vm_execute(struct state *state, struct func *fn);

#endif /* VM_H */
