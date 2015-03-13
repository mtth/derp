#include "test_cbuf.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


void test_cbuf_stream() {

  char buf[BUFSIZ];

  int fds[2];
  if (pipe(fds)) {
    assert(0);
  }

  cbuf_t *cbuf_p = cbuf_new(14);
  assert(!cbuf_size(cbuf_p));

  char *s1 = "hi";
  size_t l1 = 3;
  write(fds[1], s1, l1);
  cbuf_write(cbuf_p, fds[0], l1);
  assert(cbuf_size(cbuf_p) == 3);
  cbuf_read(cbuf_p, fds[1], l1);
  read(fds[0], buf, l1);
  assert(!cbuf_size(cbuf_p));
  assert(!strncmp(s1, buf, 3));

  char *s2 = "hello, ";
  size_t l2 = 7; // Stop before null byte.
  write(fds[1], s2, l2);
  cbuf_write(cbuf_p, fds[0], l2);
  assert(cbuf_size(cbuf_p) == l2);

  char *s3 = "world!";
  size_t l3 = 7;
  write(fds[1], s3, l3);
  cbuf_write(cbuf_p, fds[0], l3);
  assert(cbuf_size(cbuf_p) == l2 + l3);

  cbuf_read(cbuf_p, fds[1], l2 + l3);
  read(fds[0], buf, l2 + l3);
  assert(!strncmp("hello, world!", buf, l2 + l3));

  char *s4 = "a very long string that doesn't fit";
  size_t l4 = 50;
  write(fds[1], s4, l4);
  assert(cbuf_write(cbuf_p, fds[0], l4) == -1);
  assert(!cbuf_size(cbuf_p));
  read(fds[0], buf, l4);
  assert(!strncmp(s4, buf, l4));

  cbuf_del(cbuf_p);

}

void test_cbuf_copy() {

  char buf[BUFSIZ];

  int fds[2];
  if (pipe(fds)) {
    assert(0);
  }

  cbuf_t *cbuf_p = cbuf_new(14);
  assert(!cbuf_size(cbuf_p));

  char *s1 = "hi";
  size_t l1 = 3;
  cbuf_load(cbuf_p, s1, l1);
  assert(cbuf_size(cbuf_p) == 3);
  cbuf_read(cbuf_p, fds[1], l1);
  read(fds[0], buf, l1);
  assert(!cbuf_size(cbuf_p));
  assert(!strncmp(s1, buf, 3));

  char *s2 = "hello, world!";
  char l2 = 14;
  char *s2p = malloc(l2 * sizeof *s2p);
  cbuf_load(cbuf_p, s2, l2);
  cbuf_save(cbuf_p, s2p, l2);
  assert(!strncmp(s2, s2p, l2));

  cbuf_del(cbuf_p);

}

void test_cbuf() {

  printf("testing cbuf\n");
  test_cbuf_stream();
  test_cbuf_copy();

}
