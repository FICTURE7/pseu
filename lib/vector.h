#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

struct vector {
	size_t capacity;
	size_t count;
	void **items;
};

void vector_init(struct vector *vec);

void vector_deinit(struct vector *vec);

void vector_add(struct vector *vec, void *item);

void vector_remove(struct vector *vec, int index);

void *vector_get(struct vector *vec, int index);

void vector_set(struct vector *vec, int index, void *item);

#endif