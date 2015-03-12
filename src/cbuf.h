#ifndef CBUF_H_
#define CBUF_H_

#include <sys/types.h>

typedef struct cbuf cbuf_t;

cbuf_t *cbuf_new(size_t max_size);
size_t cbuf_size(cbuf_t *cbuf_p);
ssize_t cbuf_write(cbuf_t *cbuf_p, int fd, size_t max_bytes);
ssize_t cbuf_read(cbuf_t *cbuf_p, int fd, size_t max_bytes);
void cbuf_del(cbuf_t *cbuf_p);

#endif
