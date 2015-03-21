#ifndef DLIST_H_
#define DLIST_H_

/**
 * Doubly-linked list.
 *
 */

#include <sys/types.h>

struct dlist_iter {
  void *val;
  struct dlist_iter *prev, *next;
};
typedef struct dlist_iter dlist_iter_t;
typedef struct dlist dlist_t;

dlist_t *dlist_new(void); // Create new list.
size_t dlist_size(dlist_t *p); // Current size.
dlist_iter_t *dlist_insert(dlist_t *p, ssize_t n, void *val); // Insert new element at position n.
dlist_iter_t *dlist_get(dlist_t *p, ssize_t n); // Retrieve element.
void dlist_remove(dlist_t *p, dlist_iter_t *iter); // Remove element.
void dlist_del(dlist_t *p); // Destroy list.

#define dlist_next(p) (p)->next->val == NULL ? NULL : (p)->next
#define dlist_prev(p) (p)->prev->val == NULL ? NULL : (p)->prev

#endif
