#ifndef PSEU_VALUE_H
#define PSEU_VALUE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <pseu.h>
#include "pseu_opcode.h"
#include "pseu_location.h"

/* TODO: Consider implementing pointers. */
/* TODO: Review portability of flexible arrays. */

struct state;

/* 
 * Types of pseu values.
 */
enum value_type {
	VALUE_TYPE_VOID,
	VALUE_TYPE_BOOLEAN,
	VALUE_TYPE_INTEGER,
	VALUE_TYPE_REAL,
	VALUE_TYPE_OBJECT
};

/* 
 * Represents a pseu value.
 */
struct value {
	/* Type of value. */
	enum value_type type;
	
	/* Actual value. */
	union {
		/* Value as a boolean. */
		bool as_bool;
		/* Value as an int. */
		int as_int;
		/* Value as a float. */
		float as_float;
		/* Value as a pseu object. */
		struct object *as_object;
	};
};

/* 
 * Represents a pseu type.
 */
struct type {
	/* Type identifier. */
	const char *ident;
	/* Number of fields the type has. 0 if type is a primitive type. */
	size_t fields_count;
};

/* 
 * Represents a pseu variable.
 */
struct variable {
	/* Identifier of the variable. */
	const char *ident;
	/* Type of the variable. */
	struct type *type;
};

/*
 * Represents a pseu closure.
 */
struct closure {	
	/* Function which defined this closure. */
	struct function *fn;
	
	/*
	 * The amount of virtual machine stack space which should be available 
	 * before executing this function.
	 */
	size_t stack_size; 

	/* Number of constants in the function. */
	uint8_t consts_count; 
	/* Constants in the function. */
	struct value *consts; 

	/* Number of local variables in the function. */
	uint8_t locals_count; 
	/* Variables in the function. */
	struct variable *locals;

	/* Instructions of the function. */
	code_t *code;
};

/*
 * Types of pseu function.
 */
enum function_type {
	/* Primitive function. */
	FN_TYPE_PRIMITIVE,
	/* User-defined function (closure). */
	FN_TYPE_USER
};

/*
 * Represents a primitive pseu function.
 */
typedef void (*function_primitive_t)(struct state *state, struct value *args);

/*
 * Represents a pseu function.
 */
struct function {
	/* Type of the function. */
	enum function_type type;

	/* Identifier of the function. */
	const char *ident;
	/* Number of parameters of the function or arity. */
	uint8_t params_count;
	/* Array of parameter types. */
	struct type **params_types;
	/* Return type of the function. `NULL` to indicate procedure. */
	struct type *return_type;

	union {
		/* Function as a closure. */
		struct closure *as_closure;
		/* Function as a primitive function. */
		function_primitive_t as_primitive;
	};
};

/* 
 * Represents a pseu instance object which is heap allocated.
 *
 * TODO: Implement garbage collector state.
 */
struct object {
	/* Type of the pseu object. */
	struct type *type;
};

/* 
 * TODO: Consider the type of array_object->{from,to} & string_object->len,
 * for portability (full blown kappa).
 */

/* 
 * Represents a pseu array and its range.
 */
struct array_object {
	/* Object base. */
	struct object base;
	/* Start index of the array. */
	unsigned int from;
	/* End index of array. */
	unsigned int to;
	/* Elements in the array. */
	struct value items[];
};

/* 
 * Represents a pseu string object. 
 */
struct string_object {
	/* Object base. */
	struct object base;
	/* Hash of string; used for interning. */
	unsigned int hash;
	/* Length of string. */
	size_t length;
	/* Null terminated string value. */
	const char buffer[];
};

/* 
 * Represents a user defined object.
 */
struct user_object {
	/* Object base. */
	struct object base; 
	/* Field values of the object, which is based on its type. */
	struct value fields[];
};

bool value_is_number(struct value *value);
bool value_is_string(struct value *value);
bool value_is_array(struct value *value);

/*
 * 
 */
struct type *value_get_type(pseu_vm_t *vm, struct value *value);

#endif /* PSEU_VALUE_H */
