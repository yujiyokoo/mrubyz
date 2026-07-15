#include "test.h"
#include "../mrubyz.h"

extern const uint8_t load_literal_str[];
extern const uint8_t str_len[];

void test_load_literal_str() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, load_literal_str);

  Assert(T_STRING == v.type, "type is not string");
  Assert(strcmp(v.u.strval, "hello") == 0, "literal string load failed");
}

// op_string must record the pool string's length in the value
void test_string_value_carries_length() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, str_len);

  Assert(T_STRING == v.type, "str_len fixture should return a string");
  Assert(5 == v.u.str.len, "u.str.len should be the pool length (5)");
  Assert(0 == strcmp("abcde", v.u.str.ptr), "ptr should alias strval");
}

int test_string()
{
  suite_setup("string tests");
  suite_add_test(test_load_literal_str);
  suite_add_test(test_string_value_carries_length);

  return suite_run();
}
