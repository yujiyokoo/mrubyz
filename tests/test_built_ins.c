#include "test.h"
#include "../mrubyz.h"

extern const uint8_t str_built_ins[];

void test_string_built_ins() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, str_built_ins);

  Assert(T_TRUE == v.type, "string built-in returned unexpected type");
}

int test_built_ins() {
  suite_setup("built-in tests");
  suite_add_test(test_string_built_ins);

  return suite_run();
}
