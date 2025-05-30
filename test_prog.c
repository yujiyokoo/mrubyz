#include <sms.h>
#include <stdio.h>
#include "mrubyz.h"

unsigned char pal1[] = {0x00, 0x20, 0x08, 0x28, 0x02, 0x22, 0x0A, 0x2A,
  0x15, 0x35, 0x1D, 0x3D, 0x17, 0x37, 0x1F, 0x3F};

unsigned char pal2[] = {0x00, 0x03, 0x08, 0x28, 0x02, 0x22, 0x0A, 0x2A,
  0x15, 0x35, 0x1D, 0x3D, 0x17, 0x37, 0x1F, 0x3F};

extern uint8_t bytecode[];
void main() {
  int y = 0;

  clear_vram();
  load_tiles(standard_font, 0, 255, 1);
  load_palette(pal1, 0, 16);
  load_palette(pal2, 16, 16);
  set_vdp_reg(VDP_REG_FLAGS1, VDP_REG_FLAGS1_BIT7 | VDP_REG_FLAGS1_SCREEN);

  printf("mruby on Sega Master System\n");
  printf("calling mruby bytecode from C\n");

  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, bytecode);

  printf("\n");
  printf("returned num: %d\n", v.value);
  printf("\n");

  for (;;) {
    wait_vblank_noint();
  }
}
