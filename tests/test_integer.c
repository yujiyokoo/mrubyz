#include "test.h"
#include "../mrubyz.h"
#include <stdio.h>

extern uint8_t add[];
extern uint8_t loadi[];
extern uint8_t sub[];
extern uint8_t mul[];
extern uint8_t div[];

void test_integer_load_return() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, loadi);

  printf("returned num: %d\n", v.value);
  Assert(v.value == 5, "integer load failed");
}

void test_integer_add() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, add);

  printf("returned num: %d\n", v.value);
  Assert(v.value == 6, "integer add failed");
}

void test_integer_sub() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, sub);

  printf("returned num: %d\n", v.value);
  Assert(v.value == 1, "integer sub failed");
}

void test_integer_mul() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, mul);

  printf("returned num: %d\n", v.value);
  Assert(v.value == 4, "integer mul failed");
}

void test_integer_div() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, div);

  printf("returned num: %d\n", v.value);
  Assert(v.value == 3, "integer div failed");
}

int test_integer()
{
  suite_setup("integer tests");
  suite_add_test(test_integer_load_return);
  suite_add_test(test_integer_add);
  suite_add_test(test_integer_sub);
  suite_add_test(test_integer_mul);
  suite_add_test(test_integer_div);

  return suite_run();
}

