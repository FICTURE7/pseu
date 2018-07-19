#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include "location.h"

/* type of diagnostic */
enum diagnostic_type {
	DIAGNOSTIC_TYPE_ERROR, /* error diagnostic */
	DIAGNOSTIC_TYPE_WARNING, /* warning diagnostic */
};

/*
 * represents a diagnostic
 * which may be about a lexical error,
 * runtime error etc
 */
struct diagnostic {
	enum diagnostic_type type; /* type of diagnostic */
	struct location loc; /* location in source of the error */
	char *message; /* null terminated string message descripting the diagnostic */
};

#endif
