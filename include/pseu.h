#ifndef PSEU_H
#define PSEU_H

typedef struct pseu pseu_t;

pseu_t *pseu_alloc(void);
void pseu_free(pseu_t *pseu);
void pseu_init(pseu_t *pseu, char *path, char *src);
void pseu_eval(pseu_t *pseu);

#endif /* PSEU_H */
