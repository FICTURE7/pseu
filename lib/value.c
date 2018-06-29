#include <stdlib.h>
#include "value.h"

const struct type string_type = {
	.ident = "STRING",
	.nfields = -1 /* -1 since they are hardcoded fields */
};

const struct type array_type = {
	.ident = "ARRAY",
	.nfields = -1 /* -1 since they are hardcoded fields */
};

struct object *object_new(struct type *type) {
	return NULL;
}
