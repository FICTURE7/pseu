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
	enum value_type type; /* type of of the value */
	union {
		bool as_bool; /* value as a boolean */
		int as_int; /* value as an integer */
		float as_float; /* value as a real*/
		struct object *as_object; /* value as an object */
	};
};

/* represents an object which is under garbage collection */
struct object {
	unsigned int nref; /* number of refs */
};

/* represents an array which contains an array of values */
struct array_object {
	struct object base; /* to be an object */

	unsigned int from; /* start index of the array */
	unsigned int to; /* end index of the array */
	struct value **items; /* elements in the array */
};

/* represents a string object */
struct string_object {
	struct object base; /* to be an object */

	unsigned int hash; /* hash of the string */
	size_t len; /* length of the string */
	char *buf; /* pointer to the NULL terminated string */
};

/* represents a user defined object */
struct user_object {
	struct object base; /* to be an object */

	void *data;
};

#endif /* OBJECT_H */
