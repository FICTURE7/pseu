#ifndef OBJECT_H
#define OBJECT_H

#include <stdbool.h>

enum object_type {
	OBJECT_BOOL,
	OBJECT_INT,
	OBJECT_FLOAT,
	OBJECT_STRING
};

struct object {
	enum object_type type;
	union {
		bool as_bool;
		int as_int;
		float as_float;
		char *as_string;
	} value;
};

#endif
