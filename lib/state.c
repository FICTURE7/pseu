#include "state.h"
#include "string.h"

void state_init(struct state *state) {
	string_table_init(state->string_table);
}
