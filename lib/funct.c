#include <stdlib.h>
#include "vector.h"
#include "funct.h"

void funct_init(struct funct *fn) {
	vector_init(&fn->consts);
}

void funct_deinit(struct funct *fn) {
	vector_deinit(&fn->consts);
}
