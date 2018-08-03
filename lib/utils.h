#ifndef UTILS_H
#define UTILS_H

#if DEBUG
	#include <stdio.h>
	#define ASSERT(cond, msg) \
		do { \
			if (!(cond)) { \
				fprintf(stderr, "error: assert at (%s:%d) in %s failed: %s\n", \
						__FILE__, __LINE__, __func__, msg);	\
				abort(); \
			} \
		} while (0)

#else
	#define ASSERT(cond, msg) do {} while(0)
#endif

#endif /* UTILS_H */
