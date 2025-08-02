#include "test.h"
#include "../mrubyz.h"
#include <stdio.h>

extern uint8_t add[];
void test_integer_load_return() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, add);

  printf("returned num: %d\n", v.value);
  Assert(v.value == 1, "integer assertion failed");
}

int test_integer()
{
  suite_setup("Trivial integer test");
  suite_add_test(test_integer_load_return);

  return suite_run();
}

