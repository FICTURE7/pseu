#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "value.h"

struct user_object *object_new(struct type *type) {
	struct user_object *obj = malloc(sizeof(struct user_object) + (type->nfields * sizeof(struct value)));
	/* zero out region of memory where the field values will be */
	memset(obj->fields, 0, type->nfields * sizeof(struct value));
	obj->base.nrefs = 0;
	obj->base.type = type;

	return obj;
}

struct array_object *object_new_array(unsigned int from, unsigned int to) {
	size_t len = to - from;
	struct array_object *obj = malloc(sizeof(struct array_object) + (len * sizeof(struct value)));
	/* zero out region of memory where the items of the array will be */
	memset(obj->items, 0, len);
	obj->base.nrefs = 0;
	//obj->base.type = (struct type *)&array_type;

	return obj;
}

struct string_object *object_new_string(char *buffer, size_t length, unsigned int hash) {
	struct string_object *obj = malloc(sizeof(struct string_object) + length + 1);
	obj->base.nrefs = 0;
	//obj->base.type = (struct type *)&string_type;
	obj->buffer[length] = '\0';
	obj->length = length;
	obj->hash = hash;

	memcpy(obj->buffer, buffer, length);
	return obj;
}

inline bool value_is_number(struct value *value) {
	return value->type == VALUE_TYPE_INTEGER || value->type == VALUE_TYPE_REAL;
}

inline bool value_is_string(struct value *value) {
	//return value->type == VALUE_TYPE_OBJECT && value->as_object->type == &string_type;
	return 0;
}

inline bool value_is_array(struct value *value) {
	//return value->type == VALUE_TYPE_OBJECT && value->as_object->type == &array_type;
	return 0;
}
