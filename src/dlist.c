#include "dlist.h"
#include <assert.h>
#include <stdlib.h>


struct dlist {
  size_t size;
  struct dlist_iter *first, *last;
};

dlist_t *dlist_new(void) {

  dlist_iter_t *first, *last;

  first = malloc(sizeof *first);
  if (first == NULL) {
    goto error;
  }
  first->elem = NULL;
  first->prev = NULL;

  last = malloc(sizeof *last);
  if (last == NULL) {
    goto error_first;
  }
  last->elem = NULL;
  last->next = NULL;

  first->next = last;
  last->prev = first;

  dlist_t *p = malloc(sizeof *p);
  if (p == NULL) {
    goto error_last;
  }
  p->size = 0;
  p->first = first;
  p->last = last;

  return p;

error_last:
  free(last);
error_first:
  free(first);
error:
  return NULL;

}

static dlist_iter_t *iter_get(dlist_t *p, ssize_t index) {

  assert(p != NULL);

  dlist_iter_t *iter;
  if (index >= 0) {
    iter = p->first;
    while (index-- && iter != NULL) {
      iter = iter->next;
    }
  } else {
    iter = p->last;
    while (++index && iter != NULL) {
      iter = iter->prev;
    }
  }
  return iter;

}

size_t dlist_size(dlist_t *p) {

  assert(p != NULL);

  return p->size;

}

int dlist_insert(dlist_t *p, void *elem, ssize_t index) {

  assert(p != NULL);
  assert(elem != NULL);

  dlist_iter_t *prev_iter;
  if (index >= 0 && (size_t) index <= p->size) {
    prev_iter = iter_get(p, index);
  } else if (index < 0 && (size_t) (- index) <=  1 + p->size) {
    prev_iter = iter_get(p, index - 1);
  } else {
    return -1;
  }

  assert(prev_iter != NULL && prev_iter->next != NULL);

  dlist_iter_t *iter = malloc(sizeof *iter);
  if (iter == NULL) {
    return -1;
  }
  iter->elem = elem;
  iter->prev = prev_iter;
  iter->next = prev_iter->next;
  iter->next->prev = iter;
  prev_iter->next = iter;
  p->size++;
  return 0;

}

void *dlist_get(dlist_t *p, ssize_t index) {

  assert(p != NULL);

  dlist_iter_t *iter;
  if (index >= 0 && (size_t) index <= p->size) {
    iter = iter_get(p, index + 1);
  } else if (index < 0 && (size_t) (- index) <= 1 + p->size) {
    iter = iter_get(p, index - 1);
  } else {
    return NULL;
  }

  return iter->elem;

}

void *dlist_pop(dlist_t *p, ssize_t index) {

  assert(p != NULL);

  dlist_iter_t *iter;
  if (index >= 0 && (size_t) index <= p->size) {
    iter = iter_get(p, index + 1);
  } else if (index < 0 && (size_t) (- index) <= 1 + p->size) {
    iter = iter_get(p, index - 1);
  } else {
    return NULL;
  }

  iter->prev->next = iter->next;
  iter->next->prev = iter->prev;
  free(iter);
  p->size--;
  return iter->elem;

}

void dlist_del(dlist_t *p) {

  assert(p != NULL);
  // TODO

}

dlist_iter_t dlist_iter(dlist_t *p, enum dlist_order order) {

  assert(p != NULL);

  switch (order) {
  case DLIST_FWD:
    return *p->first;
  case DLIST_BWD:
    return *p->last;
  }

}

void *dlist_next(dlist_iter_t *iter_p) {

  assert(iter_p != NULL);
  assert(iter_p->elem == NULL);

  dlist_iter_t *next = iter_p->next;
  if (next == NULL) {
    return NULL;
  }

  iter_p->next = next->next;
  iter_p->prev = iter_p;
  return next->elem;

}

void *dlist_prev(dlist_iter_t *iter_p) {

  assert(iter_p != NULL);
  assert(iter_p->elem == NULL);

  dlist_iter_t *prev = iter_p->prev;
  if (prev == NULL) {
    return NULL;
  }

  iter_p->prev = prev->prev;
  iter_p->next = iter_p;
  return prev->elem;

}
