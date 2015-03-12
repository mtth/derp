/**
 * Derp: client.
 *
 *
 */

#include "rio.h"
#include "cbuf.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


int get_fd(struct in_addr host, unsigned short port) {

  int fd = socket(PF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    return -1;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr = host;
  if (connect(fd, (struct sockaddr *) &addr, sizeof addr) < 0) {
    close(fd);
    return -2;
  }

  return fd;

}

int loop(int fd, char *id) {

  fd_set read_set, write_set, ready_read_set, ready_write_set;

  if (rio_write(fd, id, strnlen(id, 8)) < 0) {
    goto error;
  }
  printf("connected\n");
  // TODO: wait for ok response (to be implemented as well).

  cbuf_t *cbuf_p = cbuf_new(BUFSIZ - 1);
  if (cbuf_p == NULL) {
    goto error;
  }

  FD_ZERO(&read_set);
  FD_ZERO(&write_set);
  FD_SET(STDIN_FILENO, &read_set);
  FD_SET(fd, &read_set);
  FD_SET(fd, &write_set);

  while (1) {
    ready_read_set = read_set;
    ready_write_set = write_set; // TODO: only if stuff waiting to be written.
    if (select(fd + 1, &ready_read_set, &ready_write_set, NULL, NULL) < 0) {
      printf("select error\n");
      return -2;
    }

    // if (FD_ISSET(STDIN_FILENO, &ready_read_set)) {
    //   ssize_t nb = rio_read(STDIN_FILENO, buf, BUFSIZ);
    //   printf("read %ld bytes.\n", nb);
    // }

    // if (FD_ISSET(fd, &ready_read_set)) {
    // }

    // if (FD_ISSET(fd, &ready_write_set)) {
    // }

  }

error:
  close(fd);
  return -1;

}

int main(int argc, char **argv) {

  if (argc != 4) {
    printf("usage: derp HOST PORT NAME\n");
    return -1;
  }

  struct in_addr host;
  if (!inet_aton(argv[1], &host)) {
    printf("invalid ip\n");
    return -2;
  }

  short port = (short) atol(argv[2]);
  if (!port) {
    printf("invalid port\n");
    return -3;
  }

  char *id = argv[3];
  if (strnlen(id, 8) > 8) {
    printf("invalid id\n");
    return -4;
  }

  int fd = get_fd(host, port);
  if (fd < 0) {
    printf("error while connecting");
    return fd;
  }

  return loop(fd, id);

}
