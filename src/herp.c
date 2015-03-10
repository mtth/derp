/**
 * Herp: server.
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

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

void loop(int listen_fd) {

  int conn_fd;
  socklen_t addr_len;
  struct sockaddr_in client_addr;

  while (1) {
    addr_len = sizeof client_addr;
    conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &addr_len);
    if (conn_fd < 0) {
      break;
    }
    printf("received connection\n");
    close(conn_fd);
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
