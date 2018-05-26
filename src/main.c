#include <notpseudo.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	notpseudo_t *notpseudo = notpseudo_alloc();
	notpseudo_init(notpseudo, "hello world.notpseudo", "DECLARE 1+1");
	notpseudo_free(notpseudo);

	getchar();
	return 0;
}