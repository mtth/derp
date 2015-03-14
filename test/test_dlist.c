#include "test_dlist.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>


void test_dlist(void) {

  printf("testing dlist\n");

  dlist_t *p = dlist_new();
  char *s1 = "hi";
  assert(!dlist_size(p));
  assert(!dlist_insert(p, s1, 0));
  assert(dlist_size(p) == 1);
  assert(!strncmp((char *) dlist_get(p, 0), s1, 3));

  char *s2 = "hey";
  assert(!dlist_insert(p, s2, -1));
  assert(dlist_size(p) == 2);
  assert(!strncmp((char *) dlist_pop(p, 1), s2, 4));
  assert(dlist_size(p) == 1);

  char *s3 = "you";
  assert(!dlist_insert(p, s3, 0));
  dlist_iter_t iter = dlist_iter(p, DLIST_FWD);
  assert(!strncmp((char *) dlist_next(&iter), s3, 4));
  assert(!strncmp((char *) dlist_next(&iter), s1, 3));
  assert(dlist_next(&iter) == NULL);

  dlist_del(p);

}
