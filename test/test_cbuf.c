#include "test_cbuf.h"
#include <assert.h>
#include <stdio.h>



void test_cbuf() {

  printf("testing cbuf\n");

  cbuf_t *cbuf_p = cbuf_new(10);
  assert(cbuf_get_size(cbuf_p) == 0);



}
