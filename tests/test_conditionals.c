#include "test.h"
#include "../mrubyz.h"

extern const uint8_t branching[];

void test_branching() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, branching);

  Assert(T_INT == v.type, "branching with if returned unexpected type");
  Assert(2 == v.intval, "branching with if returned unexpected value");
}

int test_conditionals()
{
  suite_setup("conditionals tests");
  suite_add_test(test_branching);

  return suite_run();
}
