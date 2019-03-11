#include <pseu.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char *path_combine(const char *base, const char *other) {
	size_t base_len = strlen(base);
	size_t other_len = strlen(other);
	int has_seperator = base[base_len - 1] == '/';

	size_t new_path_size = base_len + other_len + (has_seperator ? 1 : 2);
	char *new_path = malloc(new_path_size);
	if (!new_path) {
		return NULL;
	}

	/* Copy base of path and add seperator at end if it wasn't there. */
	memcpy(new_path, base, base_len);
	if (!has_seperator) {
		new_path[base_len++] = '/';
	}

	/* Copy other part of path and add NULL terminator. */
	memcpy(new_path + base_len, other, other_len);
	new_path[new_path_size - 1] = '\0';
	return new_path;
}

#define CHAR_BUFFER_CHUNK 4096

/* Represents a vector of char. */
struct char_buffer {
	char *data;
	size_t length;
	size_t size;
};

void buffer_init(struct char_buffer *buffer) {
	buffer->data= malloc(CHAR_BUFFER_CHUNK);
	buffer->length = 0;
	buffer->size = CHAR_BUFFER_CHUNK;
}

void buffer_deinit(struct char_buffer *buffer) {
	if (buffer) {
		free(buffer->data);
	}
}

int buffer_write(struct char_buffer *buffer, const char *data, size_t length) {
	if (length > CHAR_BUFFER_CHUNK) {
		return 1;
	}

	size_t new_length = buffer->length + length;
	if (new_length > buffer->size) {
		buffer->size *= 2;
		buffer->data = realloc(buffer->data, buffer->size);
	}
	
	memcpy(buffer->data + buffer->length, data, length);
	buffer->length = new_length;
	return 0;
}

int buffer_readfile(struct char_buffer *buffer, const char *path) {
	FILE *stream = fopen(path, "r");
	if (!stream) {
		return 1;
	}

	size_t count = 0;
	char *temp = malloc(CHAR_BUFFER_CHUNK);
	do {
		count = fread(temp, sizeof(char), CHAR_BUFFER_CHUNK, stream);
		buffer_write(buffer, temp, count);
	} while (count > 0);

	fclose(stream);
	return 0;
}

enum pseu_test_state {
	TEST_NOTRAN,
	TEST_PASSED,
	TEST_FAILED,
	TEST_ERR_PATH,
	TEST_ERR_ERRNO
};

/* Represents a pseu test, .pseut file. */
struct pseu_test {
	/* Name of test. */
	const char *name;
	/* Path to .pseut file. */
	const char *path;

	/* Source input part of test. */
	const char *input;
	/* Expected output part of test. */
	const char *expected_output;

	/* Actual output after test ran. */
	struct char_buffer output;
	/* State of test. */
	enum pseu_test_state state;
};

static void runner_print(pseu_vm_t *vm, const char *text) {
	struct pseu_test *test = pseu_vm_get_data(vm);
	buffer_write(&test->output, text, strlen(text));
}

static void *runner_alloc(pseu_vm_t *vm, size_t size) {
#ifdef TEST_CLEAN_MEM
	/* Force clean memory allocations for consisten testing. */
	return calloc(1, size);
#else
	return malloc(size);
#endif
}

static void *runner_realloc(pseu_vm_t *vm, void *ptr, size_t size) {
	return realloc(ptr, size);
}

static void runner_free(pseu_vm_t *vm, void *ptr) {
	free(ptr);
}

static void runner_onerror(pseu_vm_t *vm, enum pseu_error_type type,
				unsigned int row, unsigned int col, const char *message) {
	fprintf(stderr, "at %u:%u: error: %s.\n", row, col, message);
}

/* Represents a test runner. */
struct pseu_test_runner {
	/* Final result of test. 0 all passed; otherwise atleast 1 failed. */
	int result;
	/* Base path of tests files. */
	const char *base_path;
};

/* Allocates a new pseu_test instance using the specified runner and path. */
struct pseu_test *test_create(struct pseu_test_runner *runner,
					const char *path) {
	struct pseu_test *test = calloc(1, sizeof(struct pseu_test));
	test->name = malloc(strlen(path) + 1);
	test->path = path_combine(runner->base_path, path);

	strcpy((char *)test->name, path);
	buffer_init(&test->output);
	return test;
}

#define SEPERATOR "\n---\n"
#define SEPERATOR_LEN sizeof(SEPERATOR) - 1

/* 
 * Parses the specified buffer (which is not NULL terminated) into a pseu_test
 * looking for the source code section and expected output section.
 */
void test_parse(struct pseu_test *test, struct char_buffer *buffer) {
	/* Indicates if the .pseut has an expected output section. */
	int has_output = 0;

	/* Look for seperator of expected output section in .pseut file. */
	char *ptr;
	const char *buffer_start = buffer->data;
	const char *buffer_end = buffer->data + buffer->length - 1;
	for (ptr = (char *)buffer_start; ptr < buffer_end; ptr++) {
		if (!strncmp(ptr, SEPERATOR, SEPERATOR_LEN)) {
			has_output = 1;
			break;
		}
	}

	char *input_start = (char *)buffer_start;
	size_t input_length = ptr - input_start;
	char *expected_output_start;
	size_t expected_output_length;

	/* If expected output section, set section start and length. */
	if (has_output) {
		expected_output_start = ptr + SEPERATOR_LEN;
		expected_output_length = buffer_end - expected_output_start;
	}

	/* Copy sections to test->input and test->expected_output if present. */
	char *input = malloc(input_length + 1);
	memcpy(input, input_start, input_length);
	input[input_length] = '\0';
	test->input = input;
	
	if (has_output) {
		char *expected_output = malloc(expected_output_length + 1);
		memcpy(expected_output, expected_output_start, expected_output_length);
		expected_output[expected_output_length] = '\0';
		test->expected_output = expected_output;
	}

	//printf("input: \n%s\n\n", input);
	//printf("output: \n%s\n\n", expected_output);
}

/* Parse a .pseut file into a struct pseu_test. */
struct pseu_test *test_load(struct pseu_test_runner *runner,
					const char *path) {
	struct pseu_test *test = test_create(runner, path);
	/* Check if combination of path failed. */
	if (!test->path) {
		test->state = TEST_ERR_PATH;
		goto exit;
	}

	/* Initialize buffer which will contain the .pseut file content. */
	struct char_buffer buffer;
	buffer_init(&buffer);	

	/* Read the file into the buffer. */
	if (buffer_readfile(&buffer, test->path)) {
		test->state = TEST_ERR_ERRNO;
		goto exit;
	}

	test_parse(test, &buffer);
	/* Clean up file buffer after we're done with it. */
	buffer_deinit(&buffer);
exit:
	return test;
}

void test_print_state(struct pseu_test *test) {
	switch (test->state) {
		case TEST_FAILED:
			printf("\x1B[31mfailed\x1B[0m");
			break;
		case TEST_PASSED:
			printf("\x1B[32mpassed\x1B[0m");
			break;
		case TEST_ERR_PATH:
			printf("\x1B[31mfailed\x1B[0m: Unable to combine path.");
			break;
		case TEST_ERR_ERRNO:
			printf("\x1B[31mfailed\x1B[0m: %s.", strerror(errno));
			break;

		default:
			printf("unknown");
			break;
	}
}

void test_free(struct pseu_test *test) {
	if (!test) {
		return;
	}

	if (test->name) {
		free((char *)test->name);
	}
	if (test->path) {
		free((char *)test->path);
	}
	if (test->input) {
		free((char *)test->input);
	}
	if (test->expected_output) {
		free((char *)test->expected_output);
	}
	if (test->output.data) {
		buffer_deinit(&test->output);
	}

	free(test);
}

void test(struct pseu_test_runner *runner, const char *path) {
	struct pseu_test *test = test_load(runner, path);
	/* Print test name first & force a flush of the stdout. */
	printf("\x1B[33mtest\x1B[0m %s:\n", test->name);
	fflush(stdout);

	/* If test not in NOTRAN state, means that loading failed. */
	if (test->state != TEST_NOTRAN) {
		runner->result = 1;
		goto finalize;
	}

	/* Runner configuration. */
	pseu_config_t config = {
		.print = runner_print,
		.alloc = runner_alloc,
		.realloc = runner_realloc,
		.free = runner_free,
		.onerror = runner_onerror,
		.onwarn = NULL
	};

	pseu_vm_t *vm = pseu_vm_new(&config);
	pseu_vm_set_data(vm, test);
	int result = pseu_interpret(vm, test->input);
	pseu_vm_free(vm);

	if (result == PSEU_RESULT_SUCCESS) {
		if (!test->expected_output) {
			test->state = TEST_PASSED;
			goto finalize;
		}

		const char *actual = test->output.data;
		const char *expected = test->expected_output;
		size_t actual_length = test->output.length;
		size_t expected_length = strlen(test->expected_output);

		if (actual_length == expected_length && 
			!strncmp(actual, expected, actual_length)) {
			test->state = TEST_PASSED;
		} else {
			test->state = TEST_FAILED;
			runner->result = 1;
		}
	} else {
		test->state = TEST_FAILED;
		runner->result = 1;
	}

finalize:
	printf(" - ");
	test_print_state(test);
	printf("\n");
	test_free(test);
}

int main(int argc, const char **argv) {
	/* TODO: Enable ANSI color codes when on Windows. */

	if (argc < 2) {
		fprintf(stderr, "error: no test directory provided\n");
		fprintf(stderr, "usage: libpseu-test <test-directory>\n");
		return 1;
	}
	
	/* Initialize test runner. */
	struct pseu_test_runner runner = {
		.base_path = argv[1],
		.result = 0
	};

	clock_t start = clock();

	/* Run tests. */
	test(&runner, "core/output.pseut");
	test(&runner, "core/declare.pseut");
	test(&runner, "core/assign.pseut");
	test(&runner, "core/function.pseut");
	//test(&runner, "core/sandbox.pseut");

	clock_t end = clock();
	double duration = (double)(end - start) / CLOCKS_PER_SEC;

	printf("\nall tests done in %f ms -> ", duration * 1000);
	if (runner.result) {
		printf("failed\n");
	} else {
		printf("passed\n");
	}

	return runner.result;
}
