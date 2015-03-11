#ifndef HERP_H_
#define HERP_H_

#include <sys/types.h>

typedef struct herp herp_t;

herp_t herp_new(void);
int herp_add_fd(herp_t *herp_p, int fd);
fd_set herp_get_readable_fds(herp_t *herp_p);
fd_set herp_get_writable_fds(herp_t *herp_p);
int herp_read_fd(herp_t *herp_p, int fd);
int herp_write_fd(herp_t *herp_p, int fd);
void herp_remove_fd(herp_t *herp_p, int fd);
void herp_del(herp_t *herp_p);

#endif
