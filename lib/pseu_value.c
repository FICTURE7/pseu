#include <stdbool.h>

#include "pseu_vm.h"
#include "pseu_debug.h"
#include "pseu_value.h"

bool value_is_number(struct value *value) {
	return value->type == VALUE_TYPE_INTEGER || value->type == VALUE_TYPE_REAL;
}

bool value_is_string(struct value *value) {
	pseu_assert(1);
	/* TODO: Implement. */
	return false;
}

bool value_is_array(struct value *value) {
	pseu_assert(1);
	/* TODO: Implement. */
	return false;
}

struct type *value_get_type(pseu_vm_t *vm, struct value *value) {
	pseu_assert(vm && value);

	/* Otherwise find the corresponding something somehting. */
	switch (value->type) { 
		case VALUE_TYPE_BOOLEAN:
			return &vm->boolean_type;
		case VALUE_TYPE_INTEGER:
			return &vm->integer_type;
		case VALUE_TYPE_REAL:
			return &vm->real_type;
		case VALUE_TYPE_OBJECT:
			/* If value is a heap allocated object, return the object's type. */
			return value->as_object->type;
	}

	return NULL;
}
