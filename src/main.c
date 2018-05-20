#include <notpseudo.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	notpseudo_t *notpseudo = notpseudo_alloc();
	notpseudo_init(notpseudo, "hello world.notpseudo", "OUTPUT\nOUTPUT");
	notpseudo_free(notpseudo);

	getchar();
	return 0;
}