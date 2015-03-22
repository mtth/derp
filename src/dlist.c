#include "dlist.h"
#include <assert.h>
#include <stdlib.h>


struct dlist {
  size_t size;
  struct dlist_iter *iter;
};

dlist_t *dlist_new(void) {

  dlist_iter_t *iter = malloc(sizeof *iter);
  if (iter == NULL) {
    goto error;
  }
  iter->val = NULL;
  iter->next = iter->prev = iter;

  dlist_t *p = malloc(sizeof *p);
  if (p == NULL) {
    goto error_iter;
  }
  p->size = 0;
  p->iter = iter;

  return p;

error_iter:
  free(iter);
error:
  return NULL;

}

/* Helper. No size checking, can return flag element. */
static dlist_iter_t *iter_get(dlist_t *p, ssize_t n) {

  assert(p != NULL);

  dlist_iter_t *iter = p->iter;
  if (n >= 0) {
    while (n--) {
      iter = iter->next;
    }
  } else {
    while (n++) {
      iter = iter->prev;
    }
  }
  return iter;

}

size_t dlist_size(dlist_t *p) {

  assert(p != NULL);

  return p->size;

}

dlist_iter_t *dlist_insert(dlist_t *p, ssize_t n, void *val) {

  assert(p != NULL);
  assert(val != NULL);

  if (
    (n >= 0 && (size_t) n > p->size) ||
    (n < 0 && (size_t) (- n) >  1 + p->size)
  ) {
    return NULL;
  }

  dlist_iter_t *iter = malloc(sizeof *iter);
  if (iter == NULL) {
    return NULL;
  }
  iter->val = val;

  dlist_iter_t *prev = iter_get(p, n);
  iter->prev = prev;
  iter->next = prev->next;
  prev->next->prev = iter;
  prev->next = iter;
  p->size++;
  return iter;

}

dlist_iter_t *dlist_get(dlist_t *p, ssize_t n) {

  assert(p != NULL);

  dlist_iter_t *iter;
  if (n >= 0 && (size_t) n < p->size) {
    iter = iter_get(p, n + 1);
  } else if (n < 0 && (size_t) (- n) <= 1 + p->size) {
    iter = iter_get(p, n);
  } else {
    return NULL;
  }

  return iter;

}

void dlist_remove(dlist_t *p, dlist_iter_t *iter) {

  assert(iter != NULL);

  iter->prev->next = iter->next;
  iter->next->prev = iter->prev;
  free(iter);
  p->size--;

}

void dlist_del(dlist_t *p) {

  assert(p != NULL);

  dlist_iter_t *iter = p->iter->next;
  while (iter->val) {
    iter = iter->next;
    free(iter->prev);
  }
  free(iter);
  free(p);

}
