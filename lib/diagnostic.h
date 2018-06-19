#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include "vector.h"
#include "location.h"

enum diagnostic_type {
	DIAGNOSTIC_TYPE_ERROR,
	DIAGNOSTIC_TYPE_WARNING,
};

struct diagnostic {
	enum diagnostic_type type;
	struct location loc;
	char *message;
};

#endif
