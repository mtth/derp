#include "test_dlist.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void test_dlist(void) {

  printf("testing dlist\n");

  dlist_t *p = dlist_new();
  char *s1 = "hi";
  assert(!dlist_size(p));
  assert(dlist_insert(p, 0, s1));
  assert(dlist_size(p) == 1);
  assert(!strncmp((char *) dlist_get(p, 0)->val, s1, 3));

  char *s2 = "hey";
  assert(dlist_insert(p, -1, s2));
  assert(dlist_size(p) == 2);
  dlist_iter_t *iter = dlist_get(p, 1);
  assert(!strncmp((char *) iter->val, s2, 4));
  dlist_remove(p, iter);
  assert(dlist_size(p) == 1);

  char *s3 = "hello";
  assert(!dlist_insert(p, 2, s3));
  assert(!dlist_insert(p, -3, s3));
  assert(dlist_insert(p, -2, s3));
  assert(dlist_insert(p, -1, s2));
  assert(dlist_size(p) == 3);
  iter = dlist_get(p, 0);
  assert(!strncmp((char *) iter->val, s3, 6));
  iter = dlist_next(iter);
  assert(!strncmp((char *) iter->val, s1, 3));
  iter = dlist_next(iter);
  assert(!strncmp((char *) iter->val, s2, 4));
  iter = dlist_prev(iter);
  assert(!strncmp((char *) iter->val, s1, 3));

  dlist_del(p);

}
