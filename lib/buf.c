#include "pseu_vm.h"

int cbuf_new(pseu_state_t *s, struct cbuffer *buf, size_t size)
{
	buf->count = 0;
	buf->size = size;
	return pseu_vec_init(s, (void **)&buf->buffer, buf->size, sizeof(char));
}

void cbuf_free(pseu_state_t *s, struct cbuffer *buf)
{
	buf->count = 0;
	buf->size = 0;
	pseu_free(s, buf->buffer);
}

int cbuf_put(pseu_state_t *s, struct cbuffer *buf, char c)
{
	if (buf->count >= buf->size && 
			pseu_vec_grow(s, (void **)&buf->buffer, &buf->size, sizeof(char))) {
		return 1;
	}
	buf->buffer[buf->count++] = c;
	return 0;
}
