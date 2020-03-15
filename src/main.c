#include <pseu.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
	PseuVM *vm = pseu_vm_new(NULL);
	pseu_vm_eval(vm, "OUTPUT 1");
	pseu_vm_free(vm);

	return 0;
}
