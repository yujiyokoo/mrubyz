#include "test.h"
#include "../mrubyz.h"

extern const uint8_t gt_int[];
extern const uint8_t eq_int[];
extern const uint8_t eq_str[];

void test_gt_int() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, gt_int);

  Assert(T_TRUE == v.type, "comparison did not rutern true");
}

void test_eq_int() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, eq_int);

  Assert(T_TRUE == v.type, "comparison did not rutern true");
}

void test_eq_str() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, eq_str);

  Assert(T_TRUE == v.type, "comparison did not rutern true");
}

int test_compare()
{
  suite_setup("comparison tests");
  suite_add_test(test_gt_int);
  suite_add_test(test_eq_int);
  suite_add_test(test_eq_str);

  return suite_run();
}
