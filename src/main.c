#include <pseu.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
	struct pseu *pseu = pseu_new(NULL);

	pseu_interpret(pseu, "OUTPUT \"xD\"");
	pseu_free(pseu);
	return 0;
}
