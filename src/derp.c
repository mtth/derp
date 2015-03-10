/**
 * Derp: client.
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>

int usage(void) {

  printf("usage: derp HOST PORT NAME\n");
  return 1;

}

int main(int argc, char **argv) {

  if (argc != 4) {
    return usage();
  }

  short port = (short) atol(argv[1]);
  if (!port) {
    return usage();
  }

  char *host = argv[1];
  char *name = argv[3];

  return 0;

}
