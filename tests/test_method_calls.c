#include <stdio.h>
#include "test.h"
#include "../mrubyz.h"

extern const uint8_t method_calls[];

int test_foo() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, method_calls);

  Assert(v.u.intval == 8, "Unexpected return value");
}

int test_method_calls() {
  suite_setup("method call tests");
  suite_add_test(test_foo);

  return suite_run();
}
