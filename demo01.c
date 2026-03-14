#include <sms.h>
#include <stdio.h>
#include "mrubyz.h"

const unsigned char pal1[] = {0x00, 0x3F, 0x08, 0x28, 0x02, 0x22, 0x0A, 0x2A,
  0x15, 0x35, 0x1D, 0x3D, 0x17, 0x37, 0x1F, 0x3F};

const unsigned char pal2[] = {0x00, 0x03, 0x08, 0x28, 0x02, 0x22, 0x0A, 0x2A,
  0x15, 0x35, 0x1D, 0x3D, 0x17, 0x37, 0x1F, 0x3F};

// extern unsigned char _heap;
long _heap;

void check_heap() {
  unsigned int total, largest;
  mallinfo(&total, &largest);
  printf("Heap: %u total, %u largest\r", total, largest);
}

// empty method just to make the build pass. Don't call this
void show_logo() {
}

extern const uint8_t bytecode[];
void main() {
  int y = 0;

  // Manually initialize heap with available memory
  mallinit();
  sbrk(&_heap, 4096);  // Register 4KB starting at _heap

  clear_vram();
  load_tiles(standard_font, 0, 255, 1);
  load_palette(pal1, 0, 16);
  load_palette(pal2, 16, 16);
  set_vdp_reg(VDP_REG_FLAGS1, VDP_REG_FLAGS1_BIT7 | VDP_REG_FLAGS1_SCREEN);
  check_heap();

  printf("\rmruby on Sega Master System\r\r");
  printf("      demo 01\r\r");
  printf("calling mruby bytecode from C:\r\r");
  printf("a = 2\rb = 4\rc = a + b\rreturn c\r");

  char *test = malloc(10);
  printf("test is: 0x%x\r", test);

  check_heap();

  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, bytecode);
  check_heap();

  printf("\r");
  printf("returned num: %d\r", v.u.intval);
  printf("\r");

  for (;;) {
    wait_vblank_noint();
  }
}
