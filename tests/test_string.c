#include "test.h"
#include "../mrubyz.h"
#include <stdio.h>

extern uint8_t load_literal_str[];

void test_load_literal_str() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, load_literal_str);

  Assert(T_STRING == v.type, "type is not string");
  Assert(strcmp(v.strval, "hello") == 0, "literal string load failed");
}

int test_string()
{
  suite_setup("string tests");
  suite_add_test(test_load_literal_str);

  return suite_run();
}
