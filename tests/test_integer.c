#include "test.h"
#include "../mrubyz.h"
#include <stdio.h>

extern const uint8_t add[];
extern const uint8_t loadi_n[];
extern const uint8_t loadi[];
extern const uint8_t loadineg[];
extern const uint8_t sub[];
extern const uint8_t mul[];
extern const uint8_t div[];

extern const uint8_t addi[];
extern const uint8_t subi[];

void test_integer_loadi_n() {
  printf("\n\ntest_integer_loadi_n\n");
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, loadi_n);

  Assert(v.u.intval == 5, "integer load failed");
}

void test_integer_loadi() {
  printf("\n\ntest_integer_loadi\n");
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, loadi);

  Assert(v.u.intval == 127, "integer loadi failed");
}

void test_integer_loadineg() {
  printf("\n\ntest_integer_loadineg\n");
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, loadineg);

  Assert(v.u.intval == -128, "integer loadi failed");
}

void test_integer_add() {
  printf("\n\ntest_integer_add\n");
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, add);

  Assert(v.u.intval == 6, "integer add failed");
}

void test_integer_addi() {
  printf("\n\ntest_integer_add\n");
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, addi);

  Assert(v.u.intval == 6, "integer addi failed");
}

void test_integer_subi() {
  printf("\n\ntest_integer_subi\n");
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, subi);

  Assert(v.u.intval == 1, "integer subi failed");
}

void test_integer_sub() {
  printf("\n\ntest_integer_sub\n");
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, sub);

  Assert(v.u.intval == 1, "integer sub failed");
}

void test_integer_mul() {
  printf("\n\ntest_integer_mul\n");
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, mul);

  Assert(v.u.intval == 4, "integer mul failed");
}

void test_integer_div() {
  printf("\n\ntest_integer_div\n");
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, div);

  Assert(v.u.intval == 3, "integer div failed");
}

int test_integer()
{
  suite_setup("integer tests");
  suite_add_test(test_integer_loadi_n);
  suite_add_test(test_integer_loadi);
  suite_add_test(test_integer_loadineg);
  suite_add_test(test_integer_add);
  suite_add_test(test_integer_sub);
  suite_add_test(test_integer_mul);
  suite_add_test(test_integer_div);

  suite_add_test(test_integer_addi);
  suite_add_test(test_integer_subi);

  return suite_run();
}

