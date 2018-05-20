#ifndef TEST_H
#define TEST_H

#include <string.h>

#define TEST_INIT()							\
	printf(">> running tests\n------------\n")

#define TEST_DEINIT()						\
	printf("------------\n>> done\n")

#define TEST(x)								\
	printf(">>> running: %s", #x);			\
	if (x()) {								\
		printf(" -> failed\n");				\
	} else {								\
		printf(" -> success\n");			\
	}

#define TEST_ASSERT(x)						\
	if (x != 1) return 1;

#define TEST_EQ(x, y) \
	TEST_ASSERT(x == y)

#endif