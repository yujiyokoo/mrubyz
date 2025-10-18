#include "test_integer.h"
#include "test_string.h"
#include "test_compare.h"
#include "test_loop.h"

int main(int argc, char *argv[])
{
  int  res = 0;

  res += test_integer();
  res += test_string();
  res += test_compare();
  res += test_loop();

  return res;
}
