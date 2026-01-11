#include <SMSlib.h>
#include <sms.h>
#include <stdio.h>
#include "mrubyz.h"

long _heap;

const unsigned char pal1[] = {0x00, 0x3F, 0x08, 0x28, 0x02, 0x22, 0x0A, 0x2A,
  0x15, 0x35, 0x1D, 0x3D, 0x17, 0x37, 0x1F, 0x3F};

const unsigned char pal2[] = {0x00, 0x03, 0x08, 0x28, 0x02, 0x22, 0x0A, 0x2A,
  0x15, 0x35, 0x1D, 0x3D, 0x17, 0x37, 0x1F, 0x3F};

extern const uint8_t demo04_bytecode[];
void main() {
  int y = 0;

  sbrk(&_heap, 4096);  // Register 4KB starting at _heap

  SMS_VRAMmemset(0x0000, 0x00, 16384);
  load_tiles(standard_font, 0, 255, 1);
  SMS_loadBGPalette(pal1);
  SMS_loadSpritePalette(pal2);
  SMS_displayOn();

  printf("loading presentation..\r");

  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, demo04_bytecode);

  for (;;) {
    SMS_waitForVBlank();
  }
}
