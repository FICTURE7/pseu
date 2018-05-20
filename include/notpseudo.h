#ifndef NOTPSEUDO_H
#define NOTPSEUDO_H

typedef struct notpseudo notpseudo_t;

typedef struct notpseudo_err notpseudo_err_t;

notpseudo_t *notpseudo_alloc(void);

void notpseudo_free(notpseudo_t *notpseudo);

void notpseudo_init(notpseudo_t *notpseudo, char *path, char *src);

void notpseudo_eval(notpseudo_t *notpseudo);

#endif
