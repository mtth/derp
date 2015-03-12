/**
 * Herp: server.
 *
 *
 */

#include "cbuf.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


#define MAX_ID_SIZE 8
#define BUFFER_SIZE 1024

struct derp { // Internal server's client connection representation.
  int fd;
  char *id;
  cbuf_t *read_buf, *write_buf;
  struct derp *prev, *next;
};
typedef struct derp derp_t;

struct {
  fd_set fds;
  derp_t head, tail;
} herp;


int usage(void) {

  printf("usage: herp PORT\n");
  return 1;

}

/**
 * Setup the listening socket and return the corresponding descriptor.
 *
 */
int get_fd(unsigned short port) {

  int fd = socket(PF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    return -1;
  }

  int optval = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) < 0) {
    close(fd);
    return -2;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(fd, (struct sockaddr *) &addr, sizeof addr) < 0) {
    close(fd);
    return -3;
  }

  if (listen(fd, 5) < 0) {
    close(fd);
    return -4;
  }

  return fd;

}

/**
 * Retrieve a client from a descriptor.
 *
 */
derp_t *get_derp(int fd) {

  derp_t *derp_p = herp.head.next;
  while (derp_p != NULL || derp_p->fd != fd) {
    derp_p = derp_p->next;
  }
  return derp_p;

}

/**
 * Register a new client.
 *
 */
derp_t *add_derp(int fd) {

  if (fd < 0) {
    goto error_fd;
  }

  unsigned char id_len;
  if (read(fd, &id_len, 1) < 0) {
    goto error_fd;
  }

  char *id = malloc((id_len + 1) * sizeof *id);
  if (id == NULL) {
    goto error_fd;
  }

  if (read(fd, id, id_len) < 0) {
    goto error_id;
  }

  cbuf_t *read_buf = cbuf_new(BUFFER_SIZE);
  if (read_buf == NULL) {
    goto error_id;
  }

  cbuf_t *write_buf = cbuf_new(BUFFER_SIZE);
  if (write_buf == NULL) {
    goto error_read_buf;
  }

  derp_t *derp_p = malloc(sizeof *derp_p);
  if (derp_p == NULL) {
    goto error_write_buf;
  }

  derp_p->fd = fd;
  derp_p->id = id;
  derp_p->read_buf = read_buf;
  derp_p->write_buf = write_buf;

  FD_SET(fd, &herp.fds);
  derp_p->prev = herp.tail.prev;
  herp.tail.prev->next = derp_p;
  herp.tail.prev = derp_p;

  return derp_p;

error_write_buf:
  cbuf_del(write_buf);
error_read_buf:
  cbuf_del(read_buf);
error_id:
  free(id);
error_fd:
  close(fd);
  return NULL;

}

/**
 * Deregister a client and free its resources.
 *
 */
int remove_derp(int fd) {

  derp_t *derp_p = get_derp(fd);
  if (derp_p == NULL) {
    return -1;
  }

  derp_p->next->prev = derp_p->prev;
  derp_p->prev->next = derp_p->next;

  cbuf_del(derp_p->read_buf);
  cbuf_del(derp_p->write_buf);
  free(derp_p->id);
  close(derp_p->fd);
  free(derp_p);

  return 0;

}

int loop(int fd) {

  int conn_fd;
  derp_t *derp_p;
  struct sockaddr_in client_addr;
  socklen_t addr_len;
  fd_set readable_fds, writable_fds;

  herp.head.next = &herp.tail;
  herp.tail.prev = &herp.head;

  while (1) {

    readable_fds = writable_fds = herp.fds;
    FD_SET(fd, &readable_fds);
    if (select(fd + 1, &readable_fds, &writable_fds, NULL, NULL) < 0) {
      goto error;
    }

    // Handle new connection.
    if (FD_ISSET(fd, &readable_fds)) {
      addr_len = sizeof client_addr;
      conn_fd = accept(fd, (struct sockaddr *) &client_addr, &addr_len);
      derp_p = add_derp(conn_fd);
      if (derp_p == NULL) {
        printf("failed client connection");
      } else {
        printf("new connection from %s\n", derp_p->id);
      }
    }

    // Check for new client messages.

    // Send any buffered messages.

  }

  return 0;

error:
  close(fd);
  return -1;

}

int main(int argc, char **argv) {

  if (argc != 2) {
    return usage();
  }

  unsigned short port = (unsigned short) atol(argv[1]);
  if (!port) {
    return usage();
  }

  int fd = get_fd(port);
  if (fd < 0) {
    return fd;
  }

  return loop(fd);

}
