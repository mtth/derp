#ifndef DLIST_H_
#define DLIST_H_

/**
 * Doubly-linked list.
 *
 */

#include <sys/types.h>


struct dlist_iter {
  void *elem;
  struct dlist_iter *prev, *next;
};
typedef struct dlist_iter dlist_iter_t;
typedef struct dlist dlist_t;
enum dlist_order {DLIST_FWD, DLIST_BWD};

dlist_t *dlist_new(void);
size_t dlist_size(dlist_t *p);
int dlist_insert(dlist_t *p, void *elem, ssize_t index);
void *dlist_get(dlist_t *p, ssize_t index);
void *dlist_pop(dlist_t *p, ssize_t index);
void dlist_del(dlist_t *p);

dlist_iter_t dlist_iter(dlist_t *p, enum dlist_order order);
void *dlist_next(dlist_iter_t *iter_p);
void *dlist_prev(dlist_iter_t *iter_p);

#endif
