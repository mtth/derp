/**
 * Herp: server.
 *
 *
 */

#include "cbuf.h"
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
  DERP_OK,
  DERP_CLOSING
};

struct derp { // Internal server's client connection representation.
  enum derp_status status;
  int fd;
  char *id;
  cbuf_t *recv_buf, *send_buf;
  struct derp *prev, *next;
};
typedef struct derp derp_t;

struct {
  fd_set readable, writable;
  derp_t head, tail;
} herp;


/**
 * Setup the listening socket and return the corresponding descriptor.
 *
 */
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

/**
 * Retrieve a client from a descriptor.
 *
 */
derp_t *derp_find(int fd) {

  assert(fd > 0);

  derp_t *derp_p = herp.head.next;
  while (derp_p != NULL || derp_p->fd != fd) {
    derp_p = derp_p->next;
  }

  return derp_p;

}

/**
 * Register a new client.
 *
 * Note that this doesn't set its ID.
 *
 */
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

  derp_t *derp_p = malloc(sizeof *derp_p);
  if (derp_p == NULL) {
    goto error_send_buf;
  }

  derp_p->status = DERP_STARTING;
  derp_p->fd = fd;
  derp_p->id = NULL;
  derp_p->recv_buf = recv_buf;
  derp_p->send_buf = send_buf;

  FD_SET(fd, &herp.readable);
  derp_p->next = &herp.tail;
  derp_p->prev = herp.tail.prev;
  herp.tail.prev->next = derp_p;
  herp.tail.prev = derp_p;

  return derp_p;

error_send_buf:
  cbuf_del(send_buf);
error_recv_buf:
  cbuf_del(recv_buf);
error_fd:
  close(fd);
  return NULL;

}

/**
 * De-register a client and free its resources.
 *
 */
void derp_del(derp_t *derp_p) {

  assert(derp_p != NULL && derp_p->status == DERP_CLOSING);

  derp_p->next->prev = derp_p->prev;
  derp_p->prev->next = derp_p->next;

  FD_CLR(derp_p->fd, &herp.readable);
  FD_CLR(derp_p->fd, &herp.writable);
  cbuf_del(derp_p->recv_buf);
  cbuf_del(derp_p->send_buf);
  close(derp_p->fd);
  if (derp_p->id != NULL) {
    free(derp_p->id);
  }
  free(derp_p);

}

void derp_broadcast(char *msg, size_t len) {

  derp_t *derp_p = herp.head.next;
  while (derp_p->next != NULL) {
    if (cbuf_load(derp_p->send_buf, msg, len) < 0) {
      derp_p->status = DERP_CLOSING;
    } else {
      FD_SET(derp_p->fd, &herp.writable);
    }
    derp_p = derp_p->next;
  }

}

/**
 * "Callback" for when a client is readable.
 *
 */
void derp_on_readable(derp_t *derp_p) {

  assert(derp_p != NULL);

  if (derp_p->status == DERP_STARTING) {
    unsigned char id_len;
    if (read(derp_p->fd, &id_len, 1) < 0) {
      goto error;
    }
    char *derp_id = malloc((id_len + 1) * sizeof *derp_id);
    if (derp_id == NULL) {
      goto error;
    }
    if (read(derp_p->fd, derp_id, id_len + 1) < id_len + 1) {
      // TODO: allow receiving ID over several reads.
      free(derp_id);
      goto error;
    }
    derp_p->id = derp_id;
    derp_p->status = DERP_OK;
    printf("new client: %s\n", derp_id);
  } else if (derp_p->status == DERP_OK) {
    ssize_t n = cbuf_write(derp_p->recv_buf, derp_p->fd, BUFSIZ);
    // printf("received %ld bytes from %s\n", n, derp_p->id);
    if (n > 0) {
      char buf[BUFSIZ];
      cbuf_save(derp_p->recv_buf, buf, n);
      derp_broadcast(buf, (size_t) n);
    } else { // Disconnect.
      goto error;
    }
  } else {
    assert(0);
  }

  return;

error:
  derp_p->status = DERP_CLOSING;

}

/**
 * "Callback" for when a client is writable.
 *
 * This should only ever happen if the client has data waiting to be sent.
 *
 */
void derp_on_writable(derp_t *derp_p) {

  assert(
    derp_p != NULL &&
    derp_p->status == DERP_OK &&
    cbuf_size(derp_p->send_buf)
  );

  cbuf_t *buf = derp_p->send_buf;
  if (cbuf_read(buf, derp_p->fd, cbuf_size(buf)) < 0) {
    goto error;
  }
  if (!cbuf_size(buf)) {
    FD_CLR(derp_p->fd, &herp.writable);
  }
  return;

error:
  derp_p->status = DERP_CLOSING;

}

/**
 * Remove closing clients.
 *
 */
void derp_cleanup() {

  derp_t *a, *b;
  a = herp.head.next;
  while (a != NULL) {
    b = a->next;
    if (a->status == DERP_CLOSING) {
      derp_del(a);
    }
    a = b;
  }

}

/**
 * Main select loop.
 *
 */
int loop(int fd) {

  assert(fd > 0);

  int conn_fd;
  derp_t *derp_p;
  struct sockaddr_in client_addr;
  socklen_t addr_len;
  fd_set readable_fds, writable_fds;

  herp.head.next = &herp.tail;
  herp.tail.prev = &herp.head;

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
      derp_p = derp_new(conn_fd);
      if (derp_p == NULL) {
        printf("failed client connection\n");
      } else {
        printf("new connection\n"); // TODO: add IP.
      }
    }

    // Send any buffered messages.
    derp_p = herp.head.next;
    while (derp_p != NULL) {
      if (FD_ISSET(derp_p->fd, &writable_fds)) {
        derp_on_writable(derp_p);
      }
      derp_p = derp_p->next;
    }

    // Check for new client messages.
    derp_p = herp.head.next;
    while (derp_p != NULL) {
      if (FD_ISSET(derp_p->fd, &readable_fds)) {
        derp_on_readable(derp_p);
      }
      derp_p = derp_p->next;
    }

    derp_cleanup(); // Unregister clients marked for deletion.

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
