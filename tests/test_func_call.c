#include "test.h"
#include "../mrubyz.h"

extern const uint8_t function_simple[];

void test_function_simple() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, function_simple);

  Assert(T_INT == v.type, "func_call returned unexpected type");
  Assert(3 == v.u.intval, "func_call returned unexpected value");
}

int test_func_call()
{
  suite_setup("function call tests");
  suite_add_test(test_function_simple);

  return suite_run();
}
