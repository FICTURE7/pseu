#include <notpseudo.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
	notpseudo_t *notpseudo = notpseudo_alloc();
	notpseudo_init(notpseudo, "hello world.notpseudo", "OUTPUT ((1+1))\nWTF");
	notpseudo_free(notpseudo);

	getchar();
	return 0;
}
