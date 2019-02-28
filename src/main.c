#include <pseu.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
	pseu_vm_t *vm = pseu_vm_new(NULL);
	pseu_interpret(vm, "OUTPUT 1");
	pseu_vm_free(vm);

	return 0;
}
