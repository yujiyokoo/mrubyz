#include <sms.h>
#include <stdio.h>
#include "mrubyz.h"

long _heap;

const unsigned char pal1[] = {0x00, 0x3F, 0x08, 0x28, 0x02, 0x22, 0x0A, 0x2A,
  0x15, 0x35, 0x1D, 0x3D, 0x17, 0x37, 0x1F, 0x3F};

const unsigned char pal2[] = {0x00, 0x03, 0x08, 0x28, 0x02, 0x22, 0x0A, 0x2A,
  0x15, 0x35, 0x1D, 0x3D, 0x17, 0x37, 0x1F, 0x3F};

// empty method just to make the build pass. Don't call this
void show_logo() {}

extern const uint8_t demo02_bytecode[];
void main() {
  sbrk(&_heap, 4096);  // Register 4KB starting at _heap

  clear_vram();
  load_tiles(standard_font, 0, 255, 1);
  load_palette(pal1, 0, 16);
  load_palette(pal2, 16, 16);
  set_vdp_reg(VDP_REG_FLAGS1, VDP_REG_FLAGS1_BIT7 | VDP_REG_FLAGS1_SCREEN);

  printf("\rmruby demo 02 on Master System\r\r");

  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, demo02_bytecode);

  for (;;) {
    wait_vblank_noint();
  }
}
