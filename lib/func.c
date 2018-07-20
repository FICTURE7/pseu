#include <stdlib.h>
#include "func.h"

void func_init(struct func *fn) {
	/* space */
}

void func_deinit(struct func *fn) {
	free(fn->consts);
}
