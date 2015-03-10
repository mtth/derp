/**
 * Derp: client.
 *
 *
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "rio.h"


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

void loop(int fd) {

  char buf[BUFSIZ];
  fd_set read_set, write_set, ready_read_set, ready_write_set;

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
      return;
    }

    if (FD_ISSET(STDIN_FILENO, &ready_read_set)) {
      ssize_t nb = rio_read(STDIN_FILENO, buf, BUFSIZ);
      printf("read %ld bytes.\n", nb);
    }

    if (FD_ISSET(fd, &ready_read_set)) {
    }

    if (FD_ISSET(fd, &ready_write_set)) {
    }

  }

}

int main(int argc, char **argv) {

  if (argc != 4) {
    printf("usage: derp HOST PORT NAME\n");
    return -1;
  }

  short port = (short) atol(argv[1]);
  if (!port) {
    printf("invalid port\n");
    return -2;
  }

  struct in_addr host;
  if (!inet_aton(argv[2], &host)) {
    printf("invalid ip\n");
    return -3;
  }

  int fd = get_fd(host, port);
  if (fd < 0) {
    printf("error while connecting");
    return fd;
  }

  loop(fd);

}
