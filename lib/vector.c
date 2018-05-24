#include <string.h>
#include "vector.h"

#define VECTOR_INIT_CAPACITY 16

static void resize(struct vector *vec, int capacity) {
	void **items = realloc(vec->items, sizeof(void *) * capacity);
	if (items) {
		vec->items = items;
		vec->capacity = capacity;
	}
}

void vector_init(struct vector *vec) {
	vec->capacity = VECTOR_INIT_CAPACITY;
	vec->count = 0;
	vec->items = malloc(sizeof(void *) * VECTOR_INIT_CAPACITY);
}

void vector_deinit(struct vector *vec) {
	free(vec->items);
}

void vector_add(struct vector *vec, void *item) {
	if (vec->count == vec->capacity) {
		resize(vec, vec->capacity * 2);
	}
	vec->items[vec->count++] = item;
}

void vector_remove(struct vector *vec, int index) {
	if (index < 0 || index >= vec->count) {
		return;
	}
	void *dst = vec->items + index;
	void *src = vec->items + index + 1;
	size_t size = sizeof(void *) * --vec->count;
	memcpy(dst, src, size);
	if (vec->count == vec->capacity / 2) {
		resize(vec, vec->capacity / 2);
	}
}

void *vector_get(struct vector *vec, int index) {
	if (index < 0 || index >= vec->count) {
		return NULL;
	}
	return vec->items[index];
}

void vector_set(struct vector *vec, int index, void *item) {
	if (index < 0 || index >= vec->count) {
		return;
	}
	vec->items[index] = item;
}