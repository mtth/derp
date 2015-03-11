#ifndef CBUF_H_
#define CBUF_H_

#include <sys/types.h>

typedef struct cbuf cbuf_t;

cbuf_t *cbuf_new(size_t max_size);
size_t cbuf_get_size(cbuf_t *cbuf_p);
ssize_t cbuf_read_from_fd(cbuf_t *cbuf_p, int fd, size_t count);
ssize_t cbuf_write_to_fd(cbuf_t *cbuf_p, int fd, size_t count);
void cbuf_del(cbuf_t *cbuf_p);

#endif
