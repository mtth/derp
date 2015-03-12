#include <errno.h>
#include "rio.h"


typedef ssize_t io_op_t(int, void *, size_t);

static inline ssize_t rio(io_op_t op, int fd, void *buf, size_t max_bytes) {

  size_t tot_bytes = 0;
  ssize_t cur_bytes;

  while (tot_bytes < max_bytes) {
    cur_bytes = op(fd, buf, max_bytes);
    if (cur_bytes < 0) {
      if (errno == EINTR) {
        cur_bytes = 0;
      } else {
        return -1 * tot_bytes;
      }
    } else if (cur_bytes == 0) { // EOF.
      break;
    }
    tot_bytes += cur_bytes;
    buf += cur_bytes;
  }

  return tot_bytes;

}

ssize_t rio_read(int fd, void *buf, size_t max_bytes) {

  return rio(read, fd, buf, max_bytes);

}

ssize_t rio_write(int fd, const void *buf, size_t max_bytes) {

  return rio((io_op_t *) write, fd, (void *) buf, max_bytes);

}
