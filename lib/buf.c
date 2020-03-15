#include "vm.h"

int cbuf_new(State *s, CBuffer *buf, size_t size)
{
	buf->count = 0;
	buf->size = size;
	return pseu_vec_init(s, (void **)&buf->buffer, buf->size, sizeof(char));
}

void cbuf_free(State *s, CBuffer *buf)
{
	buf->count = 0;
	buf->size = 0;
	pseu_free(s, buf->buffer);
}

int cbuf_put(State *s, CBuffer *buf, char c)
{
	if (buf->count >= buf->size && 
			pseu_vec_grow(s, (void **)&buf->buffer, &buf->size, sizeof(char))) {
		return 1;
	}
	buf->buffer[buf->count++] = c;
	return 0;
}
