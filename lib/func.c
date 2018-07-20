#include <stdlib.h>
#include "vector.h"
#include "func.h"

void func_init(struct func *fn) {
	vector_init(&fn->consts);
}

void func_deinit(struct func *fn) {
	vector_deinit(&fn->consts);
}
