/**
 * Herp: server.
 *
 *
 */

#include "cbuf.h"
#include "dlist.h"
#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

enum derp_status {
  DERP_STARTING,
  DERP_WAITING,
  DERP_RECEIVING,
  DERP_CLOSING
};

struct derp {
  enum derp_status status;
  int fd;
  char *id;
  cbuf_t *recv_buf, *send_buf;
};
typedef struct derp derp_t;

struct {
  fd_set readable, writable;
  dlist_t *derps;
} herp;


// Setup the listening socket and return the corresponding descriptor.
int get_fd(unsigned short port) {

  int fd = socket(PF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    goto error;
  }

  int optval = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) < 0) {
    goto error_fd;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (bind(fd, (struct sockaddr *) &addr, sizeof addr) < 0) {
    goto error_fd;
  }

  if (listen(fd, 5) < 0) {
    goto error_fd;
  }

  return fd;

error_fd:
  close(fd);
error:
  return -1;

}

// Create a new client. Note that this doesn't set its ID or register it.
derp_t *derp_new(int fd) {

  assert(fd > 0);

  cbuf_t *recv_buf = cbuf_new(BUFFER_SIZE);
  if (recv_buf == NULL) {
    goto error_fd;
  }

  cbuf_t *send_buf = cbuf_new(BUFFER_SIZE);
  if (send_buf == NULL) {
    goto error_recv_buf;
  }

  derp_t *derp = malloc(sizeof *derp);
  if (derp == NULL) {
    goto error_send_buf;
  }

  derp->status = DERP_STARTING;
  derp->fd = fd;
  derp->id = NULL;
  derp->recv_buf = recv_buf;
  derp->send_buf = send_buf;

  FD_SET(fd, &herp.readable);

  return derp;

error_send_buf:
  cbuf_del(send_buf);
error_recv_buf:
  cbuf_del(recv_buf);
error_fd:
  close(fd);
  return NULL;

}

// Destroy a client and free its resources.
void derp_del(derp_t *derp) {

  assert(derp != NULL && derp->status == DERP_CLOSING);

  FD_CLR(derp->fd, &herp.readable);
  FD_CLR(derp->fd, &herp.writable);
  cbuf_del(derp->recv_buf);
  cbuf_del(derp->send_buf);
  close(derp->fd);
  if (derp->id != NULL) {
    free(derp->id);
  }
  free(derp);

}

void derp_broadcast(char *msg, size_t len) {

  dlist_iter_t *iter = dlist_get(herp.derps, 0);
  derp_t *derp;
  while (iter != NULL) {
    derp = iter->val;
    if (cbuf_load(derp->send_buf, msg, len) < 0) {
      derp->status = DERP_CLOSING;
    } else {
      FD_SET(derp->fd, &herp.writable);
    }
    iter = dlist_next(iter);
  }

}

/**
 * "Callback" for when a client is readable.
 *
 */
void derp_on_readable(derp_t *derp) {

  assert(derp != NULL);

  if (derp->status == DERP_STARTING) {
    unsigned char id_len;
    if (read(derp->fd, &id_len, 1) < 0) {
      goto error;
    }
    char *derp_id = malloc((id_len + 1) * sizeof *derp_id);
    if (derp_id == NULL) {
      goto error;
    }
    if (read(derp->fd, derp_id, id_len + 1) < id_len + 1) {
      // TODO: allow receiving ID over several reads.
      free(derp_id);
      goto error;
    }
    derp->id = derp_id;
    derp->status = DERP_WAITING;
    printf("new client: %s\n", derp_id);
  } else if (derp->status == DERP_WAITING) {
    ssize_t n = cbuf_write(derp->recv_buf, derp->fd, BUFSIZ);
    printf("received %ld bytes from %s\n", n, derp->id);
    if (n > 0) {
      char buf[BUFSIZ];
      cbuf_save(derp->recv_buf, buf, n);
      derp_broadcast(buf, (size_t) n);
    } else { // Disconnect.
      goto error;
    }
  } else {
    assert(0);
  }

  return;

error:
  derp->status = DERP_CLOSING;

}

/**
 * "Callback" for when a client is writable.
 *
 * This should only ever happen if the client has data waiting to be sent.
 *
 */
void derp_on_writable(derp_t *derp) {

  assert(
    derp != NULL &&
    derp->status == DERP_WAITING &&
    cbuf_size(derp->send_buf)
  );

  cbuf_t *buf = derp->send_buf;
  if (cbuf_read(buf, derp->fd, cbuf_size(buf)) < 0) {
    goto error;
  }
  if (!cbuf_size(buf)) {
    FD_CLR(derp->fd, &herp.writable);
  }
  return;

error:
  derp->status = DERP_CLOSING;

}

/**
 * Main select loop.
 *
 */
int loop(int fd) {

  assert(fd > 0);

  int conn_fd;
  derp_t *derp;
  dlist_iter_t *iter;
  struct sockaddr_in client_addr;
  socklen_t addr_len;
  fd_set readable_fds, writable_fds;

  herp.derps = dlist_new();

  while (1) {

    printf("loop\n");
    readable_fds = herp.readable;
    writable_fds = herp.writable;
    FD_SET(fd, &readable_fds);
    if (select(FD_SETSIZE, &readable_fds, &writable_fds, NULL, NULL) < 0) {
      printf("select error\n");
      goto error;
    }

    // Handle new connection.
    if (FD_ISSET(fd, &readable_fds)) {
      addr_len = sizeof client_addr;
      conn_fd = accept(fd, (struct sockaddr *) &client_addr, &addr_len);
      derp = derp_new(conn_fd);
      if (derp == NULL) {
        printf("failed client connection\n");
      } else {
        dlist_insert(herp.derps, 0, derp);
        printf("new connection\n"); // TODO: add IP.
      }
    }

    // Send any buffered messages.
    iter = dlist_get(herp.derps, 0);
    while (iter != NULL) {
      derp = iter->val;
      if (FD_ISSET(derp->fd, &writable_fds)) {
        derp_on_writable(derp);
      }
      iter = dlist_next(iter);
    }

    // Check for new client messages.
    iter = dlist_get(herp.derps, 0);
    while (iter != NULL) {
      derp = iter->val;
      if (FD_ISSET(derp->fd, &readable_fds)) {
        derp_on_readable(derp);
      }
      iter = dlist_next(iter);
    }

    // Unregister clients marked for deletion.
    iter = dlist_get(herp.derps, 0);
    while (iter != NULL) {
      dlist_iter_t *next = dlist_next(iter);
      derp = iter->val;
      if (derp->status == DERP_CLOSING) {
        printf("closing connection to %s\n", derp->id);
        derp_del(derp);
        dlist_remove(herp.derps, iter);
      }
      iter = next;
    }

  }

  return 0;

error:
  close(fd);
  return -1;

}

/**
 * Entry point.
 *
 * Parse CLI arguments and start main loop.
 *
 */
int main(int argc, char **argv) {

  if (argc != 2) {
    goto error;
  }

  unsigned short port = (unsigned short) atol(argv[1]);
  if (!port) {
    goto error;
  }

  int fd = get_fd(port);
  if (fd < 0) {
    goto error;
  }

  return loop(fd);

error:
  printf("usage: herp PORT\n");
  return -1;

}
