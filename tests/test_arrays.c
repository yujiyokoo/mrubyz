#include "test.h"
#include "../mrubyz.h"

extern const uint8_t simple_array[];

void test_simple_array() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, simple_array);

  Assert(T_INT == v.type, "array returned unexpected type");
  Assert(3 == v.u.intval, "array returned unexpected value");
}

int test_arrays()
{
  suite_setup("arrays tests");
  suite_add_test(test_simple_array);

  return suite_run();
}
