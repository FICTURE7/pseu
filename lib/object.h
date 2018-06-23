#ifndef OBJECT_H
#define OBJECT_H

#include <stdlib.h>
#include <stdbool.h>

/* types of values */
enum value_type {
	VALUE_TYPE_BOOLEAN,
	VALUE_TYPE_INTEGER,
	VALUE_TYPE_REAL,
	VALUE_TYPE_OBJECT
};

/* represents a value */
struct value {
	enum value_type type;
	union {
		bool as_bool;
		int as_int;
		float as_float;
		struct object *as_object;
	};
};

/* represents an object which is under garbage collection */
struct object {
	unsigned int refc;
};

/* represents an array which contains an array of values */
struct array_object {
	struct object base;
	unsigned int from;
	unsigned int to;
	struct value **items;
};

/* represents a string object */
struct string_object {
	struct object base;
	unsigned int hash;
	size_t len;
	char *buf;
};

/* represents a user defined object */
struct user_object {
	struct object base;
	void *data;
};

#endif /* OBJECT_H */
