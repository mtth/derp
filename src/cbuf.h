#ifndef CBUF_H_
#define CBUF_H_

#include <sys/types.h>

typedef struct cbuf cbuf_t;

/**
 * Create a new circular buffer.
 *
 */
cbuf_t *cbuf_new(size_t max_size);
// TODO: make this resizable rather than fixed size.

/**
 * Number of bytes currently used in buffer.
 *
 */
size_t cbuf_size(const cbuf_t *p);

/**
 * Load `bytes` array into buffer.
 *
 * Returns the number of bytes written (i.e. `num_bytes`) if OK, and -1 if the
 * buffer doesn't have enough capacity.
 *
 */
ssize_t cbuf_load(cbuf_t *p, const char *bytes, size_t num_bytes);

/**
 * Load buffer from descriptor.
 *
 * Returns the number of bytes written (can be less than `max_bytes`, e.g. if
 * the read system call gets interrupted) if OK, and -1 if an error occurred
 * during the read or the buffer is over capacity.
 *
 */
ssize_t cbuf_write(cbuf_t *p, int fd, size_t max_bytes);

/**
 * Save buffer to `bytes` array.
 *
 * Returns the number of bytes transferred if OK, and -1 if there aren't enough
 * bytes to read in the buffer.
 *
 */
ssize_t cbuf_save(cbuf_t *p, char *bytes, size_t num_bytes);

/**
 * Read contents of buffer into a descriptor.
 *
 * Returns the total number of bytes transferred if OK, and -1 if an error
 * occurred while writing to the descriptor or there isn't enough data to read.
 *
 */
ssize_t cbuf_read(cbuf_t *p, int fd, size_t max_bytes);

/**
 * Free buffer resources.
 *
 */
void cbuf_del(cbuf_t *p);

#endif
