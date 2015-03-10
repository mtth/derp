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

  printf("connected");

  char *name = argv[3];
  rio_write(fd, name, strnlen(name, 8));

  return 0;

}
