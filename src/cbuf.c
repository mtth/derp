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

  cbuf_t *cbuf_p = malloc(sizeof *cbuf_p);
  if (cbuf_p == NULL) {
    return NULL;
  }

  char *data = malloc((max_size + 1) * sizeof *data);
  // We use one more byte to be able to distinguish between full vs. empty.
  if (data == NULL) {
    free(cbuf_p);
    return NULL;
  }

  cbuf_p->max_size = max_size;
  cbuf_p->size = 0;
  cbuf_p->start = cbuf_p->end = cbuf_p->data = data;
  return cbuf_p;

}

size_t cbuf_size(cbuf_t *cbuf_p) {

  assert(cbuf_p != NULL);

  return cbuf_p->size;

}

ssize_t cbuf_load(cbuf_t *cbuf_p, char *bytes, size_t num_bytes) {

  assert(cbuf_p != NULL);

  if (num_bytes > (cbuf_p->max_size - cbuf_p->size)) {
    return -1;
  }

  ssize_t overshoot = (
    cbuf_p->end + num_bytes - (cbuf_p->data + cbuf_p->max_size + 1)
  );
  if (overshoot <= 0) {
    memcpy(cbuf_p->end, bytes, num_bytes);
    if (overshoot) {
      cbuf_p->end += num_bytes;
    } else {
      cbuf_p->end = 0;
    }
  } else {
    memcpy(cbuf_p->end, bytes, num_bytes - overshoot);
    memcpy(cbuf_p->data, bytes + (num_bytes - overshoot), overshoot);
    cbuf_p->end = cbuf_p->data + overshoot;
  }
  cbuf_p->size += num_bytes;
  return num_bytes;

}

ssize_t cbuf_write(cbuf_t *cbuf_p, int fd, size_t max_bytes) {

  assert(cbuf_p != NULL);

  if (max_bytes > (cbuf_p->max_size - cbuf_p->size)) {
    return -1;
  }

  ssize_t overshoot = (
    cbuf_p->end + max_bytes - (cbuf_p->data + cbuf_p->max_size + 1)
  );
  ssize_t num_bytes;
  if (overshoot <= 0) {
    num_bytes = read(fd, cbuf_p->end, max_bytes);
    if (num_bytes < 0) {
      return -1;
    }
    if (overshoot) {
      cbuf_p->end += num_bytes;
    } else {
      cbuf_p->end = 0;
    }
  } else {
    num_bytes = read(fd, cbuf_p->end, max_bytes - overshoot);
    if (num_bytes < 0) {
      return -1;
    }
    cbuf_p->end += num_bytes;
    if (num_bytes == (ssize_t) (max_bytes - overshoot)) {
      ssize_t read_extra = read(fd, cbuf_p->data, overshoot);
      if (read_extra < 0) {
        return -1;
      }
      num_bytes += read_extra;
      cbuf_p->end = cbuf_p->data + read_extra;
    }
  }
  cbuf_p->size += num_bytes;
  return num_bytes;

}

ssize_t cbuf_save(cbuf_t *cbuf_p, char *bytes, size_t num_bytes) {

  assert(cbuf_p != NULL);

  if (num_bytes > cbuf_p->size) {
    return -1;
  }

  ssize_t overshoot = (
    cbuf_p->start + num_bytes - (cbuf_p->data + cbuf_p->max_size + 1)
  );
  if (overshoot <= 0) {
    memcpy(bytes, cbuf_p->start, num_bytes);
    cbuf_p->start += num_bytes;
  } else {
    memcpy(bytes, cbuf_p->start, num_bytes - overshoot);
    memcpy(bytes + num_bytes - overshoot, cbuf_p->data, overshoot);
    cbuf_p->start = cbuf_p->data + overshoot;
  }
  cbuf_p->size -= num_bytes;
  return num_bytes;

}

ssize_t cbuf_read(cbuf_t *cbuf_p, int fd, size_t max_bytes) {

  assert(cbuf_p != NULL);

  if (max_bytes > cbuf_p->size) {
    return -1;
  }

  ssize_t overshoot = (
    cbuf_p->start + max_bytes - (cbuf_p->data + cbuf_p->max_size + 1)
  );
  ssize_t num_bytes;
  if (overshoot <= 0) {
    num_bytes = write(fd, cbuf_p->start, max_bytes);
    if (num_bytes < 0) {
      return -1;
    }
    cbuf_p->start += num_bytes;
  } else {
    num_bytes = write(fd, cbuf_p->start, max_bytes - overshoot);
    if (num_bytes < 0) {
      return -1;
    }
    cbuf_p->start += num_bytes;
    if (num_bytes == (ssize_t) (max_bytes - overshoot)) {
      ssize_t written_extra = write(fd, cbuf_p->data, overshoot);
      if (written_extra < 0) {
        return -1;
      }
      num_bytes += written_extra;
      cbuf_p->start = cbuf_p->data + written_extra;
    }
  }
  cbuf_p->size -= num_bytes;
  return num_bytes;

}

void cbuf_del(cbuf_t *cbuf_p) {

  assert(cbuf_p != NULL);

  free(cbuf_p->data);
  free(cbuf_p);

}
