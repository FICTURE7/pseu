#include <pseu.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
	pseu_t *pseu = pseu_alloc();
	pseu_init(pseu, "hello world.pseu", "OUTPUT ((1+1))\nWTF");
	pseu_free(pseu);

	getchar();
	return 0;
}
