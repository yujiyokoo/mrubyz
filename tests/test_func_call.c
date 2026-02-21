#include "test.h"
#include "../mrubyz.h"

extern const uint8_t function_simple[];
extern const uint8_t recursive[];

void test_function_simple() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, function_simple);

  Assert(T_INT == v.type, "func_call returned unexpected type");
  Assert(3 == v.u.intval, "func_call returned unexpected value");
}

void test_recursive() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, recursive);

  Assert(T_INT == v.type, "recursive returned unexpected type");
  Assert(14 == v.u.intval, "recursive returned unexpected value");
}

int test_func_call()
{
  suite_setup("function call tests");
  suite_add_test(test_function_simple);
  suite_add_test(test_recursive);

  return suite_run();
}
