#include <stdio.h>
#include <notpseudo.h>
#include "node.h"
#include "lexer.h"
#include "parser.h"
#include "visitor.h"

struct notpseudo {
	struct lexer lexer;
	struct parser parser;
	struct node *root;
};

static struct object *eval_string(struct visitor *visitor, struct node_string *string) {
	struct object *str = malloc(sizeof(struct object));
	str->type = OBJECT_STRING;
	str->value.as_string = string->val;
	return str;
}

static struct object *eval_stmt_output(struct visitor *visitor, struct node_stmt_output *stmt_output) {
	struct object *val = visitor_visit(visitor, stmt_output->expr);
	switch (val->type) {
		case OBJECT_STRING:
			printf("%s", val->value.as_string);
			break;
	}
	free(val);
	return NULL;
}

notpseudo_t *notpseudo_alloc(void) {
	return malloc(sizeof(struct notpseudo));
}

void notpseudo_free(notpseudo_t *notpseudo) {
	free(notpseudo);
}

void notpseudo_init(notpseudo_t *notpseudo, char *path, char *src) {
	lexer_init(&notpseudo->lexer, path, src);
	parser_init(&notpseudo->parser, &notpseudo->lexer);

	/* parse the src right away */
	parser_parse(&notpseudo->parser, &notpseudo->root);
}

void notpseudo_eval(notpseudo_t *notpseudo) {
	struct visitor visitor;
	visitor.string = eval_string;
	visitor.stmt_output = eval_stmt_output;

	struct object *object = visitor_visit(&visitor, notpseudo->root);
}