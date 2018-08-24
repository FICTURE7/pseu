#include <pseu.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	/* configuration */
	pseu_config_t config = {
		.init_stack_size = 1,
		.max_stack_size = 1024
	};

	/* create the pseu instance */
	pseu_t *pseu = pseu_new(&config);

	/* check if we managed to allocate the pseu instance */
	if (pseu == NULL) {
		fprintf(stderr, "error: failed to allocate new pseu instance\n");
		return 1;
	}

	/* interpret some pseu code */
	//int result = pseu_interpret(pseu, "DECLARE x : STRING\nx = \"5\"\nOUTPUT 1+x");
	//int result = pseu_interpret(pseu, "DECLARE x : INTEGER\nOUTPUT x\n");
	int result = pseu_interpret(pseu, "x <- 1");
	if (result != PSEU_RESULT_SUCCESS) {
		fprintf(stderr, "error: failed to interpret code\n");
	}

	/* free the pseu instance */
	//pseu_free(pseu);
	return result;
}
