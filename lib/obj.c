#include "vm.h"
#include "obj.h"

Array *array_new(State *s, u32 cap)
{
  Array *result = pseu_gc_new(s, V(s)->array_type, cap * sizeof(Value)); /* @ovf */
  result->length = 0;
  result->capacity = cap;
  return result;
}

Array *array_push(State *s, Array *a, Value *v)
{
  Array *result = a;
  if (a->length >= a->capacity) {
    result = array_new(s, a->capacity * 2); /* @ovf */
    result->length = a->length;
    memcpy(result->items, a->items, sizeof(Value) * a->length);
  }

  result->items[result->length++] = *v;
  return result;
}

Array *array_pop(State *s, Array *a)
{
  pseu_unused(s);

  if (a->length > 0)
    a->length--;

  /* TODO: Shrink array if its worth it.*/
  return a;
}

/*
Value *array_get(State *s, Array *a, u32 i)
{
  pseu_unused(s);

  return a->values[i];
}
*/

Type *v_type(State *s, Value *v)
{
	switch (v->type) {
	case VAL_BOOL:
		return V(s)->boolean_type;
	case VAL_INT:
		return V(s)->integer_type;
	case VAL_FLOAT:
		return V(s)->real_type;
	case VAL_OBJ:
		return v->as.object->header.type;

	default:
		return NULL;
	}
}
