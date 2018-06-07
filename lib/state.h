#ifndef STATE_H
#define STATE_H

#include "string.h"

struct state {
    struct string_table *string_table;
};

void state_init(struct state *state);

#endif /* STATE_H */
