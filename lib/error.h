#ifndef ERROR_H
#define ERROR_H

enum error_type {
	ERROR_RUNTIME,
	ERROR_COMPILE
};

struct error {
	enum error_type type;
};

#endif /* ERROR_H */
