#include <SMSlib.h>
#include <sms.h>
#include "mrubyz.h"
#include "shou_circle_defs.h"
#include "shou_sprite.h"

// circle data lives in ROM bank 2 (demo/shou_data.c)
extern const unsigned char shou_circle_palette[];
extern const unsigned char shou_circle_tiles[];
extern const unsigned char shou_circle_map[];

#if SHOU_CIRCLE_BAND_COUNT != 12
#error "demo07.rb hardcodes whiteout = 13 (BAND_COUNT + 1); update it"
#endif

long _heap;

const unsigned char sprite_pal[] = {0x00, 0x00 /* stroke: black */, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// mrubyz.c externs these; demo07 never calls the builtin that loads them
const unsigned char pal1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const unsigned char pal2[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// empty methods just to make the build pass. Don't call these
void show_logo() {}
void restore_font() {}
uint8_t bytecode_at(uint8_t i) { return 0; }

extern const uint8_t demo07_bytecode[];

void main() {
  sbrk(&_heap, 2048);  // Register 2KB starting at _heap

  // Explicitly map ROM bank 2 (circle data) into slot 2 (0x8000-0xBFFF)
  *((unsigned char *)0xFFFF) = 2;

  SMS_VRAMmemset(0x0000, 0x00, 16384);
  SMS_loadTiles(shou_circle_tiles, 0, SHOU_CIRCLE_TILE_COUNT * 32);
  SMS_loadTileMapArea(0, 0, shou_circle_map, 32, 28);
  SMS_loadBGPalette(shou_circle_palette);
  SMS_loadSpritePalette(sprite_pal);
  SMS_useFirstHalfTilesforSprites(0);
  SMS_loadTiles(shou_sprite_tiles, 444, 128);

  SMS_displayOn();
  SMS_setBackdropColor(0);

  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, demo07_bytecode);
}
