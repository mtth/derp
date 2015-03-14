#include "cbuf.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


struct cbuf {
  size_t max_size, size;
  char *start, *end, *data;
};

cbuf_t *cbuf_new(size_t max_size) {

  cbuf_t *p = malloc(sizeof *p);
  if (p == NULL) {
    return NULL;
  }

  char *data = malloc((max_size + 1) * sizeof *data);
  // We use one more byte to be able to distinguish between full vs. empty.
  if (data == NULL) {
    free(p);
    return NULL;
  }

  p->max_size = max_size;
  p->size = 0;
  p->start = p->end = p->data = data;
  return p;

}

size_t cbuf_size(cbuf_t *p) {

  assert(p != NULL);

  return p->size;

}

ssize_t cbuf_load(cbuf_t *p, char *bytes, size_t num_bytes) {

  assert(p != NULL);

  if (num_bytes > (p->max_size - p->size)) {
    return -1;
  }

  ssize_t overshoot = p->end + num_bytes - (p->data + p->max_size + 1);
  if (overshoot <= 0) {
    memcpy(p->end, bytes, num_bytes);
    if (overshoot) {
      p->end += num_bytes;
    } else {
      p->end = 0;
    }
  } else {
    memcpy(p->end, bytes, num_bytes - overshoot);
    memcpy(p->data, bytes + (num_bytes - overshoot), overshoot);
    p->end = p->data + overshoot;
  }
  p->size += num_bytes;
  return num_bytes;

}

ssize_t cbuf_write(cbuf_t *p, int fd, size_t max_bytes) {

  assert(p != NULL);

  if (max_bytes > (p->max_size - p->size)) {
    return -1;
  }

  ssize_t overshoot = p->end + max_bytes - (p->data + p->max_size + 1);
  ssize_t num_bytes;
  if (overshoot <= 0) {
    num_bytes = read(fd, p->end, max_bytes);
    if (num_bytes < 0) {
      return -1;
    }
    if (overshoot) {
      p->end += num_bytes;
    } else {
      p->end = 0;
    }
  } else {
    num_bytes = read(fd, p->end, max_bytes - overshoot);
    if (num_bytes < 0) {
      return -1;
    }
    p->end += num_bytes;
    if (num_bytes == (ssize_t) (max_bytes - overshoot)) {
      ssize_t read_extra = read(fd, p->data, overshoot);
      if (read_extra < 0) {
        return -1;
      }
      num_bytes += read_extra;
      p->end = p->data + read_extra;
    }
  }
  p->size += num_bytes;
  return num_bytes;

}

ssize_t cbuf_save(cbuf_t *p, char *bytes, size_t num_bytes) {

  assert(p != NULL);

  if (num_bytes > p->size) {
    return -1;
  }

  ssize_t overshoot = p->start + num_bytes - (p->data + p->max_size + 1);
  if (overshoot <= 0) {
    memcpy(bytes, p->start, num_bytes);
    p->start += num_bytes;
  } else {
    memcpy(bytes, p->start, num_bytes - overshoot);
    memcpy(bytes + num_bytes - overshoot, p->data, overshoot);
    p->start = p->data + overshoot;
  }
  p->size -= num_bytes;
  return num_bytes;

}

ssize_t cbuf_read(cbuf_t *p, int fd, size_t max_bytes) {

  assert(p != NULL);

  if (max_bytes > p->size) {
    return -1;
  }

  ssize_t overshoot = p->start + max_bytes - (p->data + p->max_size + 1);
  ssize_t num_bytes;
  if (overshoot <= 0) {
    num_bytes = write(fd, p->start, max_bytes);
    if (num_bytes < 0) {
      return -1;
    }
    p->start += num_bytes;
  } else {
    num_bytes = write(fd, p->start, max_bytes - overshoot);
    if (num_bytes < 0) {
      return -1;
    }
    p->start += num_bytes;
    if (num_bytes == (ssize_t) (max_bytes - overshoot)) {
      ssize_t written_extra = write(fd, p->data, overshoot);
      if (written_extra < 0) {
        return -1;
      }
      num_bytes += written_extra;
      p->start = p->data + written_extra;
    }
  }
  p->size -= num_bytes;
  return num_bytes;

}

void cbuf_del(cbuf_t *p) {

  assert(p != NULL);

  free(p->data);
  free(p);

}
