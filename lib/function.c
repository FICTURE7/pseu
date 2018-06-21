#include <stdlib.h>
#include "vector.h"
#include "function.h"

void function_init(struct function *fn) {
	fn->code = NULL;
	vector_init(&fn->consts);
}
