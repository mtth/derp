#include "cbuf.h"
#include "derp.h"
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define DERP_BUF_LEN 1024

struct derp {
  char available_msgs, pending_bytes;
  cbuf_t *recv_buf, *send_buf;
};

derp_t *derp_new() {

  cbuf_t *recv_buf = cbuf_new(DERP_BUF_LEN);
  if (recv_buf == NULL) {
    goto error;
  }

  cbuf_t *send_buf = cbuf_new(DERP_BUF_LEN);
  if (send_buf == NULL) {
    goto error_recv_buf;
  }

  derp_t *derp = malloc(sizeof *derp);
  if (derp == NULL) {
    goto error_send_buf;
  }

  derp->available_msgs = 0;
  derp->pending_bytes = 0;
  derp->recv_buf = recv_buf;
  derp->send_buf = send_buf;
  return derp;

error_send_buf:
  cbuf_del(send_buf);
error_recv_buf:
  cbuf_del(recv_buf);
error:
  return NULL;

}

void derp_del(derp_t *p) {

  assert(p != NULL);

  cbuf_del(p->recv_buf);
  cbuf_del(p->send_buf);
  free(p);

}

int derp_recv_msg(derp_t *p, char *dst) {

  assert(p != NULL);
  assert(dst != NULL);

  if (!p->available_msgs) {
    return -1;
  }

  char len;
  cbuf_save(p->recv_buf, &len, 1);
  cbuf_save(p->recv_buf, dst, len);
  p->available_msgs--;
  return len;

}

int derp_send_msg(derp_t *p, char *src, char len) {

  assert(p != NULL);
  assert(src != NULL);

  // char len = strnlen(src, DERP_MAX_MSG_LEN) + 1;
  if (len > DERP_MAX_MSG_LEN) {
    return -1; // Message too long.
  }

  if (1 + len + cbuf_size(p->send_buf) > DERP_BUF_LEN) {
    return -2; // Not enough buffer space.
  }

  cbuf_load(p->send_buf, &len, 1);
  cbuf_load(p->send_buf, src, len);
  return 0;

}

int derp_on_readable_fd(derp_t *p, int fd) {

  assert(p != NULL);

  if (p->pending_bytes) {
    ssize_t n = cbuf_write(p->recv_buf, fd, p->pending_bytes);
    if (n <= 0) {
      return -1;
    }
    p->pending_bytes -= n;
  } else {
    char n;
    if (
      read(fd, &n, 1) <= 0 ||
      cbuf_load(p->recv_buf, &n, 1) < 0
    ) {
      return -1;
    }
    p->pending_bytes = n;
  }

  assert(p->pending_bytes >= 0);

  if (!p->pending_bytes) {
    p->available_msgs++;
  }
  return 0;

}

int derp_on_writable_fd(derp_t *p, int fd) {

  assert(p != NULL);

  ssize_t n = cbuf_size(p->send_buf);
  if (cbuf_read(p->send_buf, fd, n) < 0) {
    return -1;
  }

  return cbuf_size(p->send_buf);

}
