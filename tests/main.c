#include "test_integer.h"
#include "test_string.h"
#include "test_compare.h"
#include "test_loop.h"
#include "test_output.h"
#include "test_loop_output.h"
#include "test_method_calls.h"
#include "test_conditionals.h"
#include "test_arrays.h"
#include "test_func_call.h"
#include "test_built_ins.h"

int main(int argc, char *argv[])
{
  int  res = 0;

  res += test_integer();
  res += test_string();
  res += test_compare();
  res += test_loop();
  res += test_output();
  res += test_loop_output();
  res += test_method_calls();
  res += test_conditionals();
  res += test_arrays();
  res += test_func_call();
  res += test_built_ins();

  return res;
}
