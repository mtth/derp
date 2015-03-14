#ifndef CBUF_H_
#define CBUF_H_

/**
 * Circular buffer.
 *
 */

#include <sys/types.h>


typedef struct cbuf cbuf_t;

cbuf_t *cbuf_new(size_t max_size); // Instantiate new buffer.
size_t cbuf_size(cbuf_t *p); // Currently used buffer space.

// IO functions (all return the number of bytes read / written on success and
// -1 on error).
ssize_t cbuf_load(cbuf_t *p, char *bytes, size_t num_bytes); // Load from byte array.
ssize_t cbuf_write(cbuf_t *p, int fd, size_t max_bytes); // Load from descriptor.
ssize_t cbuf_save(cbuf_t *p, char *bytes, size_t num_bytes); // Save into byte array.
ssize_t cbuf_read(cbuf_t *p, int fd, size_t max_bytes); // Save to descriptor.

void cbuf_del(cbuf_t *p); // Release buffer.

#endif
