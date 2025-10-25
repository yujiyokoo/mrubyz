#include <stdio.h>
#include "test.h"
#include "../mrubyz.h"

extern uint8_t puts_str[];

// TODO: properly check the output. currently it's checked by me visually :(
int test_puts() {
  //char buffer[256];
  //FILE* buf = fmemopen(buffer, sizeof(buffer), "w");
  //fprintf(buf, "hello\n");
  //printf("You printed: %s", buffer);

  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, puts_str);

}

int test_output() {
  suite_setup("output tests");
  suite_add_test(test_puts);

  return suite_run();
}
