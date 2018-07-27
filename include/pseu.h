#ifndef PSEU_H
#define PSEU_H

/*
 * represents a pseu instance
 */
struct pseu;

/*
 * represents the configuration of a
 * pseu instance
 */
struct pseu_config {
	int stub;
};

/*
 * allocates a new pseu instance with the
 * specified configuration
 */
struct pseu *pseu_new(struct pseu_config *config);

/*
 * frees the specified pseu instance along
 * with its resources
 */
void pseu_free(struct pseu *pseu);

/*
 * interprets the specified source using
 * the specified pseu instance
 */
void pseu_interpret(struct pseu *pseu, char *src);

#endif /* PSEU_H */
