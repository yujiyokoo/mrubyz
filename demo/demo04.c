#include <SMSlib.h>
#include <sms.h>
#include <stdio.h>
#include "mrubyz.h"
#include "font_data.h"

long _heap;

extern volatile uint16_t timer;


// First colour is for the BG
// Second colour is for the text
const unsigned char pal1[] = {0x00, 0x3F, 0x01, 0x05, 0x05, 0x06, 0x06, 0x0A,
  0x3F, 0x35, 0x1D, 0x3D, 0x17, 0x37, 0x1F, 0x3F};

// The second colur is for the higlighted text
const unsigned char pal2[] = {0x00, 0x0F, 0x08, 0x03, 0x02, 0x22, 0x0A, 0x2A,
  0x15, 0x35, 0x1D, 0x3D, 0x17, 0x37, 0x1F, 0x3F};

const unsigned char enemy_tile[] = {
  0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x1b, 0x03, 0x18,
  0x00, 0x3f, 0x03, 0x24,
	0x00, 0x7f, 0x03, 0x50,
	0x00, 0xfe, 0x80, 0x4a,
  0x00, 0x24, 0x24, 0x00,
	0x00, 0x66, 0x66, 0x00
};

const unsigned char player_tile[] = {
  0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04,
  0x0e, 0x0e, 0x0e, 0x0e,
  0x1f, 0x1f, 0x1b, 0x1b,
  0xbe, 0xbe, 0xbe, 0xbf,
  0x7c, 0x7c, 0x7c, 0x7c,
  0x36, 0x36, 0x36, 0x36,
  0x63, 0x63, 0x63, 0x63
};

extern const unsigned char title_tiles[];
extern const unsigned char title_map[];

// This should be defined elesewhere...
#define TITLE_TILE_COUNT 298
void show_logo() {
  // Tile 0 is 'blank'
  load_tiles(title_tiles, 1, TITLE_TILE_COUNT, 4);
  SMS_loadTileMapArea(0, 0, title_map, 32, 24);
}

void restore_font() {
  load_tiles(demo04_font, 1, DEMO04_FONT_TILE_COUNT, 1);
}

extern const uint8_t demo04_bytecode[];
void main() {
  sbrk(&_heap, 4096);  // Register 4KB starting at _heap

  // Explicitly set bank 2 to slot 2
  *((unsigned char *)0xFFFF) = 2;

  SMS_VRAMmemset(0x0000, 0x00, 16384);
  load_tiles(demo04_font, 1, DEMO04_FONT_TILE_COUNT, 1);
  SMS_loadBGPalette(pal1);
  SMS_loadSpritePalette(pal2);

  SMS_useFirstHalfTilesforSprites(0);
  SMS_loadTiles(player_tile, 447, 32);
  SMS_loadTiles(enemy_tile, 446, 32);

  SMS_initSprites();
  SMS_finalizeSprites();
  SMS_copySpritestoSAT();

  SMS_displayOn();
  SMS_setBackdropColor(0);

  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, demo04_bytecode);
}
