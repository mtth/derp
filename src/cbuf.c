#include "cbuf.h"
#include "rio.h"
#include <stdlib.h>

struct cbuf {
  size_t max_size, size;
  char *start, *end, *data;
};

cbuf_t *cbuf_new(size_t max_size) {

  cbuf_t *cbuf_p = malloc(sizeof *cbuf_p);
  if (cbuf_p == NULL) {
    return NULL;
  }

  char *data = malloc(max_size * sizeof *data);
  if (data == NULL) {
    free(cbuf_p);
    return NULL;
  }

  cbuf_p->max_size = max_size;
  cbuf_p->size = 0;
  cbuf_p->start = cbuf_p->end = cbuf_p->data = data;

}

size_t cbuf_get_size(cbuf_t *cbuf_p) {

  return cbuf_p->size;

}

ssize_t cbuf_read_from_fd(cbuf_t *cbuf_p, int fd, size_t count) {

  if (count > (cbuf_p->max_size - cbuf_p->size)) {
    return -1;
  }

  char *start = cbuf_p->start;
  char *data = cbuf_p->data;
  ssize_t overshoot = start + count - (data + cbuf_p->size);
  ssize_t read;
  if (overshoot <= 0) {
    read = rio_read(fd, start, count);
    cbuf_p->end += read;
  } else {
    read = rio_read(fd, start, count - overshoot);
    if (read == count - overshoot) {
      ssize_t read_extra = rio_read(fd, data, overshoot);
      read += read_extra;
      cbuf_p->end = data + read_extra;
    } else {
      cbuf_p->end += read;
    }
  }
  cbuf_p->size += read;
  return read;

}

ssize_t cbuf_write_to_fd(cbuf_t *cbuf_p, int fd, size_t count) {

  if (count > cbuf_p->size) {
    return -1;
  }

  char *data = cbuf_p->data;
  ssize_t overshoot = cbuf_p->start + count - (data + cbuf_p->size);
  ssize_t written;
  if (overshoot <= 0) {
    written = rio_write(fd, cbuf_p->start, count);
    cbuf_p->start += written;
  } else {
    written = rio_write(fd, cbuf_p->start, count - overshoot);
    if (written == count - overshoot) {
      ssize_t written_extra = rio_write(fd, data, overshoot);
      written += written_extra;
      cbuf_p->start = data + written_extra;
    } else {
      cbuf_p->start += written;
    }
  }
  cbuf_p->size -= written;
  return written;

}

void cbuf_del(cbuf_t *cbuf_p) {

  free(cbuf_p->data);
  free(cbuf_p);

}
