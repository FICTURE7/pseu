#ifndef PSEU_VECTOR_H
#define PSEU_VECTOR_H

#include <stdlib.h>

/*
 * Represents a vector of type void * items.
 */
struct vector {
	/* Amount of items in the vector. */
	size_t count;
	/* Size/capacity of the vector. */
	size_t size;
	/* 
	 * Block of void * items in the vector. 
	 * TODO: Consider renaming this to item instead.
	 */
	void **data;
};

/*
 * Initializes the specified vector.
 *
 * @param vm Virtual machine instance to use.
 * @param vector Vector to initialize.
 * @param init_size Initail size of the vector.
 */
void vector_init(pseu_vm_t *vm, struct vector *vector, size_t init_size);

/*
 * Frees the items in the vector.
 *
 * @param vm Virtual machine instance to use.
 * @param vector Vector to deinitialize.
 */
void vector_deinit_items(pseu_vm_t *vm, struct vector *vector);

/*
 * Frees the resources used by the vector.
 *
 * @note This does not free items in the vector, use vector_deinit_items 
 * to do so.
 *
 * @param vm Virtual machine instance to use.
 * @param vector Vector to deinitialize.
 */
void vector_deinit(pseu_vm_t *vm, struct vector *vector);

/*
 * Adds the specified item to the specified vector.
 *
 * @param vm Virtual machine instance to use.
 * @param vector Vector to use.
 * @param item Item to add.
 */
void vector_add(pseu_vm_t *vm, struct vector *vector, void *item);

#endif /* PSEU_VECTOR_H */
