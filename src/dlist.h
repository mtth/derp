#ifndef DLIST_H_
#define DLIST_H_

#include <sys/types.h>

struct dlist_iter {
  void *val;
  struct dlist_iter *prev, *next;
};

typedef struct dlist_iter dlist_iter_t;
typedef struct dlist dlist_t;

/**
 * Create a new doubly-linked list.
 *
 */
dlist_t *dlist_new(void);

/**
 * Return current size of the list.
 *
 * This operation is cached for a O(1) cost.
 *
 */
size_t dlist_size(dlist_t *p);

/**
 * Insert a new element at position `n`.
 *
 * `val` cannot be null. The function returns the new list element.
 *
 */
dlist_iter_t *dlist_insert(dlist_t *p, ssize_t n, void *val);

/**
 * Retrieve the element at position `n`.
 *
 * `n` can be non-negative (yielding positions starting from the head of the
 * list, `0` being the first), or negative (yielding positions from the end,
 * `-1` being the last). Returns null if `n` is an invalid index.
 *
 */
dlist_iter_t *dlist_get(dlist_t *p, ssize_t n);

/**
 * Delete the corresponding list element.
 *
 */
void dlist_remove(dlist_t *p, dlist_iter_t *iter);
// Note that we expose this rather than a "delete by index" method to allow for
// efficient list rewiring (O(1) in this case, as opposed to the O(n) by
// index removal). This function is the reason why the `dlist_iter_t` type is
// exposed (unfortunate but practical).

/**
 * Free list resources.
 *
 * Note that this will free all list elements but won't touch any of the
 * pointers stored inside each element.
 *
 */
void dlist_del(dlist_t *p);

/**
 * Get the following element.
 *
 * Returns null if at the end of the list.
 *
 */
#define dlist_next(p) (p)->next->val == NULL ? NULL : (p)->next

/**
 * Get the previous element.
 *
 * Returns null if at the beginning of the list.
 *
 */
#define dlist_prev(p) (p)->prev->val == NULL ? NULL : (p)->prev

#endif
