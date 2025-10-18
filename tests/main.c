#include "test_integer.h"
#include "test_string.h"

int main(int argc, char *argv[])
{
  int  res = 0;

  res += test_integer();
  res += test_string();
  res += test_compare();

  return res;
}
