#include <stdio.h>
#include "test.h"
#include "../mrubyz.h"

extern const uint8_t puts_str[];
extern const uint8_t packed_text[];
extern uint8_t cursor_x;
extern uint8_t cursor_y;

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

// Decode two mask blocks: A,B,\r,C,PAD*4 then 0x1BB,PAD*7.
// Cursor trace (replicating puts semantics exactly, including the
// post-newline x=1 quirk): A(0,0) B(1,0) CR C(1,1) [pads] 0x1BB(2,1)
void test_puts_packed_decodes_blocks() {
  mrbz_val v;
  mrbz_vm vm;
  cursor_x = 0;
  cursor_y = 0;
  mrbz_vm_run(&vm, &v, packed_text);

  Assert(3 == cursor_x, "cursor_x should be 3 after A,B,CR,C,0x1BB");
  Assert(1 == cursor_y, "cursor_y should be 1 after one newline");
}

int test_output() {
  suite_setup("output tests");
  suite_add_test(test_puts);
  suite_add_test(test_puts_packed_decodes_blocks);

  return suite_run();
}
