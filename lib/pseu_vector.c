#include "pseu_vm.h"
#include "pseu_debug.h"
#include "pseu_vector.h"

void vector_init(pseu_vm_t *vm, struct vector *vector, size_t init_size) {
	pseu_assert(vm && vector);

	vector->count = 0;
	vector->size = init_size;
	vector->data = pseu_alloc(vm, sizeof(void *) * init_size);
}

void vector_deinit_items(pseu_vm_t *vm, struct vector *vector) {
	pseu_assert(vm && vector);

	for (size_t i = 0; i < vector->count; i++) {
		pseu_free(vm, vector->data[i]);
	}
}

void vector_deinit(pseu_vm_t *vm, struct vector *vector) {
	pseu_assert(vm && vector);
	pseu_free(vm, vector->data);
}

void vector_add(pseu_vm_t *vm, struct vector *vector, void *item) {
	pseu_assert(vm && vector && item);

	if (vector->count >= vector->size) {
		size_t new_size = vector->size * 2;
		vector->data = pseu_realloc(vm, vector->data, new_size);
		vector->size = new_size;
	}
	vector->data[vector->count++] = item;
}
