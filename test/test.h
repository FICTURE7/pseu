#ifndef TEST_H
#define TEST_H

#include <string.h>

#define TEST_INIT()									\
	printf(">> running tests\n------------\n")

#define TEST_DEINIT()								\
	printf("------------\n>> done\n")

#define TEST(x)										\
	printf(">>> running: %s", #x);					\
	if (x()) {										\
		printf(" -> failed\n");						\
	} else {										\
		printf(" -> success\n");					\
	}

#define TEST_ASSERT(x)								\
	if ((x) != 1) return 1;

#define TEST_EQ(x, y)								\
	TEST_ASSERT((x) == (y))

#define TEST_EQ_TOK(t, tt, tv)						\
	TEST_ASSERT(t.type == tt);						\
	TEST_ASSERT(t.len == strlen(tv));						\
	TEST_ASSERT(memcmp(tv, t.loc.pos, strlen(tv)) == 0)	\

#endif
