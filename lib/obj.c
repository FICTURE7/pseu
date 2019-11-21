#include "obj.h"

struct type *v_type(pseu_state_t *s, struct value *v)
{
	switch (v->type) {
	case VAL_BOOL:
		return VM(s)->boolean_type;
	case VAL_INT:
		return VM(s)->integer_type;
	case VAL_FLOAT:
		return VM(s)->real_type;
	case VAL_OBJ:
		return v->as.object->type;
	default:
		return NULL;
	}
}
