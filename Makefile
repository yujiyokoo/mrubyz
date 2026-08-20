# Z88DK compiler configuration
ZCC = docker run --platform linux/amd64 --rm -i -v $(PWD):/src z88dk/z88dk zcc
CFLAGS = +sms -vn -m -DAMALLOC
MRBC = mrbc
MISAKI_BDF = misaki_gothic.bdf
FONT8_BIN  = FONT8.BIN

# Check for misaki font bdf
ifeq (,$(wildcard $(MISAKI_BDF)))
  $(warning WARNING: $(MISAKI_BDF) is missing — demo04 build will fail. Please download the BDF version from https://littlelimit.net/misaki.htm and place $(MISAKI_BDF) in the root directory of this project)
endif

BUILDDIR=build

# Demo target
DEMOS = demo01 demo02 demo03 demo04 demo05 demo06 demo07

# Ruby bytecode files
BYTECODE_FILES = demo/add.ruby.c demo/demo02.ruby.c demo/demo03.ruby.c demo/demo04.ruby.c demo/demo05.ruby.c demo/demo06.ruby.c demo/demo07.ruby.c

.PHONY: all test clean ti84_01

all: $(DEMOS)

# tests
test:
	$(MAKE) -C tests

clean:
	rm -f $(BUILDDIR)/*
	rm -f $(BYTECODE_FILES)
	rm -f encoding_map.rb font_data.h $(FONT8_BIN) demo/shou_circle.h demo/shou_circle_defs.h demo/shou_sprite.h demo/shou_text.h demo/shou_text_defs.h
	$(MAKE) -C tests clean

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# demo programs
demo01: demo/demo01.c mrubyz.c demo/add.ruby.c | $(BUILDDIR)
	$(ZCC) $(CFLAGS) demo/demo01.c mrubyz.c demo/add.ruby.c -o $(BUILDDIR)/demo01 -create-app

demo/add.ruby.c: demo/add.rb
	$(MRBC) -B bytecode --verbose -o $@ $<

# TI-84 Plus test program (z88dk +ti83p, plain TI-OS `Asm(` executable).
# Output name ti8401 becomes the TI-OS program name TI8401 (8 chars max, letters and digits only).
# Not in DEMOS: `make` builds only the SMS demos; run `make ti84_01` for this one.
TI84_CFLAGS = +ti83p -subtype=asm -vn -m -DAMALLOC -DMRBZ_MINIMAL

ti84_01: demo/ti84_01.c mrubyz.c demo/add.ruby.c | $(BUILDDIR)
	$(ZCC) $(TI84_CFLAGS) demo/ti84_01.c mrubyz.c demo/add.ruby.c -o $(BUILDDIR)/ti8401 -create-app

demo02: demo/demo02.c mrubyz.c demo/demo02.ruby.c | $(BUILDDIR)
	$(ZCC) $(CFLAGS) demo/demo02.c mrubyz.c demo/demo02.ruby.c -o $(BUILDDIR)/demo02 -create-app

demo/demo02.ruby.c: demo/demo02.rb
	$(MRBC) -B demo02_bytecode --verbose -o $@ $<

demo03: demo/demo03.c mrubyz.c demo/demo03.ruby.c | $(BUILDDIR)
	$(ZCC) $(CFLAGS) demo/demo03.c mrubyz.c demo/demo03.ruby.c -o $(BUILDDIR)/demo03 -create-app

demo/demo03.ruby.c: demo/demo03.rb
	$(MRBC) -B demo03_bytecode --verbose -o $@ $<

# Extract standard_font binary from z88dk Docker image (gitignored)
$(FONT8_BIN):
	docker run --rm --platform linux/amd64 z88dk/z88dk \
	  cat /opt/z88dk/libsrc/target/sms/classic/FONT8.BIN > $@

# Font artifacts — generated from presentation text + Misaki BDF + standard_font
encoding_map.rb font_data.h: demo/demo04_data.txt $(MISAKI_BDF) $(FONT8_BIN)
	ruby scripts/font_builder.rb $< $(MISAKI_BDF) $(FONT8_BIN) encoding_map.rb font_data.h

# Convert title.png (256x192) to SMS tile/map/palette header in upper palette half (8-15)
# Strategy: dither source against the 64-color SMS palette first (so all pixels become valid
# SMS colors, with dithering creating perceived intermediate hues), then reduce to 8 colors.
# Background is a tiled brick pattern from a clean region; tiles containing text are kept unique.
# tile_offset=256 because demo04 loads the title tiles at VRAM index 256.
$(BUILDDIR)/sms_palette.ppm: scripts/gen_sms_palette.rb | $(BUILDDIR)
	ruby scripts/gen_sms_palette.rb > $@

# The navy key visual needs no colour warming (RED_SHIFT etc. were tuned for the old bricks)
demo/title.h: title.png scripts/png_to_sms_pattern.rb $(BUILDDIR)/sms_palette.ppm | $(BUILDDIR)
	convert title.png -dither FloydSteinberg -remap $(BUILDDIR)/sms_palette.ppm \
	  +dither -colors 8 PNG8:$(BUILDDIR)/small_8col.png
	convert $(BUILDDIR)/small_8col.png -depth 8 RGB:$(BUILDDIR)/small_pixels.rgb
	convert $(BUILDDIR)/small_8col.png -unique-colors -depth 8 RGB:$(BUILDDIR)/small_palette.rgb
	RED_SHIFT=0 DARKEN_R=0 DARKEN_G=0 DARKEN_B=0 ruby scripts/png_to_sms_pattern.rb $(BUILDDIR)/small_pixels.rgb $(BUILDDIR)/small_palette.rgb title 2 1 # leave 0x00 for the blank tile
	mv title.h demo/title.h

demo04: demo/demo04.c mrubyz.c demo/demo04.ruby.c font_data.h demo/title.c | $(BUILDDIR)
	$(ZCC) $(CFLAGS) demo/demo04.c mrubyz.c demo/demo04.ruby.c demo/title.c -o $(BUILDDIR)/demo04 -create-app

demo/demo04.rb: demo/demo04.src.rb demo/demo04_data.txt encoding_map.rb
	ruby scripts/converter.rb demo/demo04_data.txt > /tmp/demo04_temp.rb ; cat demo/demo04.src.rb >> /tmp/demo04_temp.rb ; mv /tmp/demo04_temp.rb demo/demo04.rb

demo/demo04.ruby.c: demo/demo04.rb
	$(MRBC) -B demo04_bytecode --verbose -o $@ $<

demo05: demo/demo05.c mrubyz.c demo/demo05.ruby.c | $(BUILDDIR)
	$(ZCC) $(CFLAGS) demo/demo05.c mrubyz.c demo/demo05.ruby.c -o $(BUILDDIR)/demo05 -create-app

demo/demo05.ruby.c: demo/demo05.rb
	$(MRBC) -B demo05_bytecode --verbose -o $@ $<

demo/demo06.mrb: demo/demo06.rb
	$(MRBC) --verbose -o $@ $<

demo/demo06_aot.c: demo/demo06.mrb scripts/aot_compiler.rb
	ruby scripts/aot_compiler.rb $< > $@

demo06: demo/demo06.c mrubyz.c demo/demo06_aot.c | $(BUILDDIR)
	$(ZCC) $(CFLAGS) -DAOT demo/demo06.c mrubyz.c demo/demo06_aot.c -o $(BUILDDIR)/demo06 -create-app

demo/shou_circle.h demo/shou_circle_defs.h: scripts/gen_shou_circle.rb
	ruby scripts/gen_shou_circle.rb demo/shou_circle.h

demo/shou_sprite.h: scripts/gen_shou_sprite.rb
	ruby scripts/gen_shou_sprite.rb $@

demo/shou_text.h demo/shou_text_defs.h: scripts/gen_shou_text.rb scripts/font_builder.rb $(FONT8_BIN)
	ruby scripts/gen_shou_text.rb $(FONT8_BIN) demo/shou_text.h

# -DSHOU_TEXT enables the demo07-only reveal_letter/hide_letters builtins in mrubyz.c
demo07: demo/demo07.c demo/shou_data.c mrubyz.c demo/demo07.ruby.c demo/shou_circle.h demo/shou_circle_defs.h demo/shou_sprite.h demo/shou_text.h demo/shou_text_defs.h | $(BUILDDIR)
	$(ZCC) $(CFLAGS) -DSHOU_TEXT demo/demo07.c demo/shou_data.c mrubyz.c demo/demo07.ruby.c -o $(BUILDDIR)/demo07 -create-app

demo/demo07.ruby.c: demo/demo07.rb
	$(MRBC) -B demo07_bytecode --verbose -o $@ $<
