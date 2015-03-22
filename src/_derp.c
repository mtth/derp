#include "cbuf.h"
#include "derp.h"
#include <assert.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Open socket and connect to server.
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

// Main loop.
int loop(int fd, char *id) {

  assert(fd > 0);

  derp_t *derp = derp_new();
  if (derp == NULL) {
    goto error;
  }
  derp_send_msg(derp, id, strnlen(id, DERP_MAX_MSG_LEN + 1)); // Register client.

  fd_set read_set, write_set, ready_read_set, ready_write_set;
  FD_ZERO(&read_set);
  FD_ZERO(&write_set);
  FD_SET(STDIN_FILENO, &read_set);
  FD_SET(fd, &read_set);
  FD_SET(fd, &write_set);

  while (1) {

    ready_read_set = read_set;
    ready_write_set = write_set;
    if (select(fd + 1, &ready_read_set, &ready_write_set, NULL, NULL) < 0) {
      printf("select error\n");
      goto error_derp;
    }

    if (FD_ISSET(STDIN_FILENO, &ready_read_set)) {
      ssize_t n;
      char buf[BUFSIZ];
      n = read(STDIN_FILENO, buf, BUFSIZ); // Read actual data.
      buf[n - 1] = '\0'; // Replace trailing newline with null byte.
      if (derp_send_msg(derp, buf, n) < 0) {
        printf("send error\n");
      } else {
        FD_SET(fd, &write_set);
      }
    }

    if (FD_ISSET(fd, &ready_read_set)) {
      char buf[DERP_MAX_MSG_LEN];
      char len;
      if (derp_on_readable_fd(derp, fd) < 0) {
        goto error_derp;
      }
      while ((len = derp_recv_msg(derp, buf)) >= 0) {
        printf("%s\n", buf);
      }
    }

    if (FD_ISSET(fd, &ready_write_set)) {
      int n = derp_on_writable_fd(derp, fd);
      if (n < 0) {
        goto error_derp;
      } else if (!n) {
        // Nothing more to write, clear bit.
        FD_CLR(fd, &write_set);
      }
    }

  }

error_derp:
  derp_del(derp);
error:
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
  if (strnlen(id, DERP_MAX_MSG_LEN + 1) > DERP_MAX_MSG_LEN) {
    printf("id too long\n");
    return -4;
  }

  int fd = get_fd(host, port);
  if (fd < 0) {
    printf("error while connecting");
    return fd;
  }

  return loop(fd, id);

}
