#include "test.h"
#include "../mrubyz.h"
#include <stdio.h>

extern uint8_t add[];

void test_integer_add() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, add);

  printf("returned num: %d\n", v.value);
  Assert(v.value == 6, "integer assertion failed");
}

int test_integer()
{
  suite_setup("integer tests");
  suite_add_test(test_integer_add);

  return suite_run();
}

