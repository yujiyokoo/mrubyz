#include "test.h"
#include "../mrubyz.h"

extern const uint8_t while_simple[];

void test_while_simple() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, while_simple);

  Assert(T_INT == v.type, "while example did not rutern int");
  Assert(1 == v.u.intval, "while example did not rutern 1");
}

int test_loop() {
  suite_setup("loop tests");
  suite_add_test(test_while_simple);

  return suite_run();
}
