#include <errno.h>
#include "rio.h"

ssize_t rio_read(int fd, void *buf, size_t n_bytes) {

  size_t total_read = 0;
  ssize_t n_read;

  while (total_read < n_bytes) {
    n_read = read(fd, buf, n_bytes);
    if (n_read < 0) {
      if (errno == EINTR) {
        n_read = 0;
      } else {
        return -1;
      }
    } else if (n_read == 0) {
      break;
    }
    total_read += n_read;
    buf += n_read;
  }

  return total_read;

}

ssize_t rio_write(int fd, const void *buf, size_t n_bytes) {

  size_t total_written = 0;
  ssize_t n_written;

  while (1) {
    n_written = write(fd, buf, n_bytes);
    if (n_written < 0 && errno != EINTR) {
      return -1;
    }
    total_written += n_written;
    if (total_written == n_bytes) {
      return 0;
    }
  }

}
