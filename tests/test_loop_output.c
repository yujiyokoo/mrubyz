#include "test.h"
#include "../mrubyz.h"

extern const uint8_t puts_in_loop[];

// TODO: properly test output. Currently checked by me visually
void test_while_puts() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, puts_in_loop);

  Assert(T_INT == v.type, "loop-output did not rutern int");
  Assert(1 == v.u.intval, "loop-output did not rutern 1");
}

int test_loop_output() {
  suite_setup("loop-output tests");
  suite_add_test(test_while_puts);

  return suite_run();
}
