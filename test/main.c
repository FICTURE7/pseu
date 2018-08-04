#include <pseu.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	pseu_t *pseu = pseu_new(NULL);

	/* check if we managed to allocate the pseu instance */
	if (pseu == NULL) {
		fprintf(stderr, "error: failed to allocate new pseu instance");
		return 1;
	}

	/* interpret some pseu code */
	int result = pseu_interpret(pseu, "OUTPUT \"hello there! from the vm!\"");
	if (result != PSEU_RESULT_SUCCESS) {
		fprintf(stderr, "error: failed to interpret code");
	}

	/* free the pseu instance */
	pseu_free(pseu);
	return result;
}
