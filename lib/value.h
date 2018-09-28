#ifndef VALUE_H
#define VALUE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "opcode.h"
#include "location.h"

/* types of values */
enum value_type {
	VALUE_TYPE_BOOLEAN,
	VALUE_TYPE_INTEGER,
	VALUE_TYPE_REAL,
	VALUE_TYPE_OBJECT,
	VALUE_TYPE_POINTER
};

/* represents a value */
struct value {
	enum value_type type; /* type of the value */
	union {
		bool as_bool; /* value as a boolean */
		int as_int; /* value as an integer */
		float as_float; /* value as a real*/
		struct object *as_object; /* value as an object */
		struct value *as_pointer; /* value as a pointer */
	};
};

/* represents the type of an object */
struct type {
	const char *ident; /* type identifier */
	size_t nfields; /* number of fields in the object */
};

/* represents a variable */
struct variable {
	char *ident; /* identifier of the variable */
	struct type *type; /* type of the variable */
};

/*
 * represents a function prototype
 */
struct proto {
	char *ident; /* identifier of the function */

	uint8_t nparams; /* parameter count or arity */
	struct type *params; /* array of parameter types */

	struct type *return_type; /* return type */
	struct location location; /* where in the source code the proto was definied */
};

/*
 * represents a function
 */
struct func {
	/* prototype of the function */
	struct proto *proto; 

	/*
	 * constant values used by the function
	 *
	 * `nconsts` indicates the number of variables
	 * in `consts`
	 */
	uint8_t nconsts; 
	struct value *consts; 

	/*
	 * local variables of the functions
	 *
	 * `nlocals` indicates the number of variables
	 * in `locals`
	 */
	uint8_t nlocals; 
	struct variable *locals;

	/*
	 * instr_t representation of the function, which
	 * will be interpreted by the vm/bytecode interpreter
	 *
	 * `ncode` indicates the number of instructions
	 * in `code`
	 */
	size_t ncode;
	code_t *code;

	/* 
	 * number of slots the function occupies on the stack,
	 * including locals and stack pushes 
	 *
	 * this is used to ensure the vm allocates enough space
	 * on the stack when the function is called
	 */
	size_t stack_size; 
};

/* 
 * represents an instance object
 * which is heap allocated
 */
struct object {
	unsigned int nrefs; /* number of refs for gc */
	struct type *type; /* type of object */
};

/* represents an array which is within a range */
struct array_object {
	struct object base; /* to be an object */
	unsigned int from; /* start index of the array */
	unsigned int to; /* end index of the array */
	struct value items[]; /* elements in the array */
};

/* represents a string object */
struct string_object {
	struct object base; /* to be an object */
	unsigned int hash; /* hash of the string */
	size_t length; /* length of the string */
	char buffer[]; /* pointer to the NULL terminated string */
};

/* represents a user defined object */
struct user_object {
	struct object base; /* to be an object */
	struct value fields[]; /* fields in the object */
};

struct user_object *object_new(struct type *type);
struct array_object *object_new_array(unsigned int from, unsigned int to);
struct string_object *object_new_string(char *buffer, size_t length, unsigned int hash);

bool value_is_number(struct value *value);
bool value_is_string(struct value *value);
bool value_is_array(struct value *value);

#endif /* VALUE_H */
