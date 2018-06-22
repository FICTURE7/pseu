#include <stdlib.h>
#include "vector.h"
#include "function.h"

void function_init(struct function *fn) {
	vector_init(&fn->consts);
}

void function_deinit(struct function *fn) {
	free(fn->code);
	vector_deinit(&fn->consts);
}
