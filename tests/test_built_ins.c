#include "test.h"
#include "../mrubyz.h"

extern const uint8_t str_built_ins[];
extern const uint8_t random_builtin[];

void test_string_built_ins() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, str_built_ins);

  Assert(T_TRUE == v.type, "string built-in returned unexpected type");
}

void test_random_built_in() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, random_builtin);

  Assert(T_INT == v.type, "random built-in returned unexpected type");
  Assert(v.u.intval >= 0 && v.u.intval < 10, "random(10) out of 0..9 range");
}

int test_built_ins() {
  suite_setup("built-in tests");
  suite_add_test(test_string_built_ins);
  suite_add_test(test_random_built_in);

  return suite_run();
}
