#include "derp.h"
#include "dlist.h"
#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

enum status {
  CLIENT_STARTING,
  CLIENT_RECEIVING,
  CLIENT_CLOSING
};

typedef struct {
  char *id;
  int fd;
  derp_t *derp;
  enum status status;
} client_t;

struct {
  fd_set readable, writable;
  dlist_t *clients;
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

void on_connection(int fd) {

  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof client_addr;
  int conn_fd = accept(fd, (struct sockaddr *) &client_addr, &addr_len);

  derp_t *derp = derp_new();
  if (derp == NULL) {
    return;
  }

  client_t *client = malloc(sizeof *client);
  if (client == NULL) {
    free(derp);
    return;
  }

  client->fd = conn_fd;
  client->derp = derp;
  client->id = NULL;
  client->status = CLIENT_STARTING;
  FD_SET(conn_fd, &herp.readable);
  dlist_insert(herp.clients, 0, client);

}

void on_readable(client_t *client) {

  assert(client != NULL);

  if (derp_on_readable_fd(client->derp, client->fd) < 0) {
    client->status = CLIENT_CLOSING;
    return;
  }

  char buf[DERP_MAX_MSG_LEN];
  char len;
  while ((len = derp_recv_msg(client->derp, buf)) >= 0) {
    if (client->status == CLIENT_STARTING) {
      // Bootstrap ID.
      char *id = malloc(len * sizeof *id);
      memcpy(id, buf, len);
      // TODO: check that this ID doesn't already exist.
      client->id = id;
      client->status = CLIENT_RECEIVING;
      printf("[%s] connected\n", id);
    } else {
      // Broadcast message.
      dlist_iter_t *iter;
      client_t *other;
      iter = dlist_get(herp.clients, 0);
      while (iter != NULL) {
        other = iter->val;
        if (other->id != client->id) {
          // TODO: add sender as prefix.
          derp_send_msg(other->derp, buf, len);
          FD_SET(other->fd, &herp.writable);
        }
        iter = dlist_next(iter);
      }
    }
  }

}

void on_writable(client_t *client) {

  assert(client != NULL);

  ssize_t n = derp_on_writable_fd(client->derp, client->fd) < 0;
  if (n < 0) {
    client->status = CLIENT_CLOSING;
  } else if (!n) {
    FD_CLR(client->fd, &herp.writable);
  }

}

int loop(int fd) {

  assert(fd > 0);

  herp.clients = dlist_new();
  FD_ZERO(&herp.readable);
  FD_SET(fd, &herp.readable);
  FD_ZERO(&herp.writable);

  while (1) {

    fd_set readable, writable;
    dlist_iter_t *iter;
    client_t *client;

    readable = herp.readable;
    writable = herp.writable;
    if (select(FD_SETSIZE, &readable, &writable, NULL, NULL) < 0) {
      printf("select error\n");
      goto error;
    }

    // Handle new connection.
    if (FD_ISSET(fd, &readable)) {
      on_connection(fd);
    }

    // Send any buffered messages.
    iter = dlist_get(herp.clients, 0);
    while (iter != NULL) {
      client = iter->val;
      if (FD_ISSET(client->fd, &writable)) {
        on_writable(client);
      }
      iter = dlist_next(iter);
    }

    // Check for new client messages.
    iter = dlist_get(herp.clients, 0);
    while (iter != NULL) {
      client = iter->val;
      if (FD_ISSET(client->fd, &readable)) {
        on_readable(client);
      }
      iter = dlist_next(iter);
    }

    // Unregister clients marked for deletion.
    iter = dlist_get(herp.clients, 0);
    while (iter != NULL) {
      dlist_iter_t *next = dlist_next(iter);
      client = iter->val;
      if (client->status == CLIENT_CLOSING) {
        printf("[%s] left\n", client->id);
        FD_CLR(client->fd, &herp.readable);
        FD_CLR(client->fd, &herp.writable);
        derp_del(client->derp);
        free(client);
        dlist_remove(herp.clients, iter);
      }
      iter = next;
    }

  }

  return 0;

error:
  close(fd);
  return -1;

}

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
