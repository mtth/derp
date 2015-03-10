#include <unistd.h>

ssize_t rio_read(int fd, void *buf, size_t n_bytes);
ssize_t rio_write(int fd, const void *buf, size_t n_bytes);
