#include "test.h"
#include "../mrubyz.h"

extern uint8_t gt_int[];

void test_gt_int() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, gt_int);

  Assert(T_TRUE == v.type, "comparison did not rutern true");
}

int test_compare()
{
  suite_setup("comparison tests");
  suite_add_test(test_gt_int);

  return suite_run();
}
