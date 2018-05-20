#include <stdlib.h>
#include "object.h"

struct object *object_new(enum object_type type) {
	struct object *object = malloc(sizeof(struct object));
	object->type = type;
	return object;
}
