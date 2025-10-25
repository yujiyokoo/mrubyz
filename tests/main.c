#include "test_integer.h"
#include "test_string.h"
#include "test_compare.h"
#include "test_loop.h"
#include "test_output.h"
#include "test_loop_output.h"

int main(int argc, char *argv[])
{
  int  res = 0;

  res += test_integer();
  res += test_string();
  res += test_compare();
  res += test_loop();
  res += test_output();
  res += test_loop_output();

  return res;
}
