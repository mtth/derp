/**
 * Herp: server.
 *
 *
 */

#include "cbuf.h"
#include "rio.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


struct derp { // Internal server's client connection representation.
  int fd;
  cbuf_t *read_buf, *write_buf;
  struct derp *prev, *next;
};
typedef struct derp derp_t;

struct {
  unsigned int n_derps;
  fd_set _rfd, _wfd;
  derp_t *derps[FD_SETSIZE];
} herp;


int usage(void) {

  printf("usage: herp PORT\n");
  return 1;

}

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

void loop(int fd) {

  char buf[BUFSIZ];
  int conn_fd;
  socklen_t addr_len;
  struct sockaddr_in client_addr;
  fd_set _read_set, read_set, _write_set, write_set;

  FD_ZERO(&read_set);
  FD_SET(fd, &read_set);
  FD_SET(fd, &write_set);

  while (1) {
    ready_set = read_set;
    if (select(fd + 1, &ready_set, NULL, NULL, NULL) < 0) {
      break;
    }
    if (FD_ISSET(STDIN_FILENO, &ready_set)) {
      ssize_t nb = rio_read(STDIN_FILENO, buf, BUFSIZ);
      printf("read %ld bytes.\n", nb);
    }
    if (FD_ISSET(fd, &ready_set)) {
      addr_len = sizeof client_addr;
      conn_fd = accept(fd, (struct sockaddr *) &client_addr, &addr_len);
      if (conn_fd < 0) {
        break;
      }
      ssize_t nb = rio_read(conn_fd, buf, BUFSIZ);
      buf[nb] = '\0';
      printf("received connection from %s\n", buf);
      close(conn_fd);
    }
  }

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

  loop(fd);

}
