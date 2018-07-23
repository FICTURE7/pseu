#include <stdlib.h>
#include <stdbool.h>
#include "value.h"

const struct type void_type = {
	.ident = "VOID",
	.nfields = -1 /* -1 since they are hardcoded fields */
};

const struct type array_type = {
	.ident = "ARRAY",
	.nfields = -1 /* -1 since they are hardcoded fields */
};

const struct type string_type = {
	.ident = "STRING",
	.nfields = -1 /* -1 since they are hardcoded fields */
};

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
	obj->base.type = &array_type;

	return obj;
}

struct string_object *object_new_string(char *buf, size_t len, unsigned int hash) {
	struct string_object *obj = malloc(sizeof(struct string_object));
	obj->base.nrefs = 0;
	obj->base.type = &string_type;
	obj->buf = malloc(len + 1);
	obj->buf[len] = '\0';
	obj->len = len;
	obj->hash = hash;

	memcpy(obj->buf, buf, len);
	return obj;
}

inline bool value_is_number(struct value *value) {
	return value->type == VALUE_TYPE_INTEGER || value->type == VALUE_TYPE_REAL;
}

inline bool value_is_string(struct value *value) {
	return value->type == VALUE_TYPE_OBJECT && value->as_object->type == &string_type;
}

inline bool value_is_array(struct value *value) {
	return value->type == VALUE_TYPE_OBJECT && value->as_object->type == &array_type;
}
