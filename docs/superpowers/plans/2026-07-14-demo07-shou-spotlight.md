# demo07 「照」Spotlight → Whiteout Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A standalone SMS ROM (demo07) where a black 「照」 sprite is chased by a scrolling white spotlight disc, and the speaker manually widens the light band-by-band until full whiteout — per `docs/superpowers/specs/2026-07-05-shou-theme-features-design.md`.

**Architecture:** All light effects are palette (CRAM) writes and sprite placement — zero per-pixel math on the Z80. A build-time Ruby script generates a concentric-band circle background (tiles + map + initial palette) as a C header; the Ruby demo drives everything through existing VM primitives (`set_colour`, `set_scroll`, `read_buttons`, `put_sprite`, …), which are all already implemented on main.

**Tech Stack:** Ruby (build scripts, minitest, mruby demo code), C (SMS harness via z88dk/SMSlib in Docker), GNU Make.

**Implementation order** (from the design doc): circle asset generator → static whiteout (phases 2–3) → 照 sprite → spotlight tracking (phase 1).

---

## Context for a zero-context engineer

- **Build:** `make demo07` will compile via Docker. The `ZCC` variable in the root `Makefile` currently passes `-t` to `docker run`, which **fails in non-TTY shells** ("the input device is not a TTY"). Task 2 removes the `-t` flag first.
- **Run/verify:** `.sms` ROMs land in `build/`. Visual behavior is verified by a human in an emulator (per the design doc's test policy, ROM demos get manual visual verification; build-script logic gets minitest).
- **Script tests:** run directly, e.g. `ruby tests/scripts/test_gen_shou_circle.rb`. They are NOT part of `make test` (that's the z80 VM test suite; run it anyway before commits to catch mrubyz.c regressions — expect `N run, N passed, 0 failed`).
- **Existing patterns to copy:** `demo/demo05.c` (C harness shape), `demo/demo05.rb` (button edge-detection idiom `btns & 0x10 == 0 && prev_btns & 0x10 != 0`), `scripts/png_to_sms_pattern.rb` (4bpp planar tile encoding), `scripts/converter.rb` + `tests/scripts/test_converter.rb` (module + `if __FILE__ == $0` + minitest convention).
- **VM Ruby subset (IMPORTANT):** no `<` / `<=` (use `>` / `==` with operands swapped), no float, no string manipulation, `while`/`if`/`unless`/`elsif`, integer `+ - * /`, bitwise `&`, `int16_t` values. Arrays exist but must NOT cross the Ruby↔VM boundary (design principle).
- **SMS facts:** BG plane is 32×28 tiles (256×224 px), screen shows 256×192; X scroll wraps mod 256, Y scroll must stay 0–223. Tilemap entries are 2-byte words: bits 0–8 tile index, bit 9 = H-flip (0x0200), bit 10 = V-flip (0x0400). BG palette = CRAM 0–15, sprite palette = 16–31; sprite palette index 0 is always transparent. With `SMS_useFirstHalfTilesforSprites(0)`, sprites use VRAM tiles 256+, and `put_sprite(x, y, t)`'s `t` is relative to 256.
- **Geometry chosen here:** circle centre (128, 96); 12 bands, radii `14, 20, 26, 32, 38, 44, 50, 56, 62, 68, 74, 80`. Outer radius 80 leaves 16 px vertical margin (96−80) and 48 px horizontal margin (128−80), so scroll clamps of ±32 (X) / ±16 (Y) can never wrap the circle around a screen edge. Band *b* is painted with palette index *b* (1 = centre disc); index 0 is the dark surround. Initial palette: index 1 = white 0x3F (the spotlight), everything else black 0x00. The inner disc must stay ≥14 px radius — it's the visible spotlight under a 16×16 sprite; do not shrink radii to save tiles.
- **VRAM tile budget:** these radii produce ~330 unique tiles (rings thinner than 8 px make most tiles straddle band boundaries). That's fine: the SMS has 448 tiles of VRAM (0x0000–0x37FF) and demo07 has no font, so BG tiles use indices 0–443 (`MAX_TILES = 440`) and the 4 照 sprite tiles are loaded at VRAM 444–447 (sprite half is 256–447, so `put_sprite` sees them as tiles 188–191).

---

### Task 1: Circle background generator (`scripts/gen_shou_circle.rb`)

**Files:**
- Create: `scripts/gen_shou_circle.rb`
- Test: `tests/scripts/test_gen_shou_circle.rb`

- [ ] **Step 1.1: Write the failing tests**

```ruby
# tests/scripts/test_gen_shou_circle.rb
require 'minitest/autorun'
require_relative '../../scripts/gen_shou_circle'

class TestBandIndex < Minitest::Test
  def test_centre_is_innermost_band
    assert_equal 1, ShouCircle.band_index(128, 96)
  end

  def test_corner_is_outside
    assert_equal 0, ShouCircle.band_index(0, 0)
  end

  def test_just_inside_outer_radius_is_last_band
    assert_equal 12, ShouCircle.band_index(128 + 80, 96)
  end

  def test_just_outside_outer_radius_is_outside
    assert_equal 0, ShouCircle.band_index(128 + 81, 96)
  end
end

class TestFlips < Minitest::Test
  def asym_tile
    t = Array.new(64, 0)
    t[0] = 1 # top-left pixel only
    t
  end

  def test_hflip_moves_topleft_to_topright
    assert_equal 1, ShouCircle.hflip(asym_tile)[7]
  end

  def test_vflip_moves_topleft_to_bottomleft
    assert_equal 1, ShouCircle.vflip(asym_tile)[56]
  end

  def test_flips_are_involutions
    assert_equal asym_tile, ShouCircle.hflip(ShouCircle.hflip(asym_tile))
    assert_equal asym_tile, ShouCircle.vflip(ShouCircle.vflip(asym_tile))
  end
end

class TestTileEncoding < Minitest::Test
  def test_solid_index1_tile_sets_only_plane0
    bytes = ShouCircle.encode_tile(Array.new(64, 1))
    assert_equal 32, bytes.length
    8.times { |row| assert_equal [0xFF, 0x00, 0x00, 0x00], bytes[row * 4, 4] }
  end

  def test_solid_index15_tile_sets_all_planes
    bytes = ShouCircle.encode_tile(Array.new(64, 15))
    8.times { |row| assert_equal [0xFF, 0xFF, 0xFF, 0xFF], bytes[row * 4, 4] }
  end
end

class TestBuildMap < Minitest::Test
  def test_map_covers_full_bg_plane
    _unique, words = ShouCircle.build_map
    assert_equal 32 * 28, words.length
  end

  def test_tile_budget
    unique, _words = ShouCircle.build_map
    assert unique.length <= ShouCircle::MAX_TILES,
           "#{unique.length} unique tiles exceeds #{ShouCircle::MAX_TILES}"
  end

  def test_map_references_valid_tiles
    unique, words = ShouCircle.build_map
    words.each { |w| assert_operator (w & 0x1FF), :<, unique.length }
  end

  def test_symmetry_uses_flips
    _unique, words = ShouCircle.build_map
    assert words.any? { |w| (w & 0x0200) != 0 }, "expected some H-flipped entries"
    assert words.any? { |w| (w & 0x0400) != 0 }, "expected some V-flipped entries"
  end

  def test_initial_palette_lights_only_the_disc
    pal = ShouCircle.initial_palette
    assert_equal 16, pal.length
    assert_equal 0x3F, pal[1]
    assert_equal 0x00, pal[0]
    (2..15).each { |i| assert_equal 0x00, pal[i] }
  end
end
```

- [ ] **Step 1.2: Run tests, verify they fail**

Run: `ruby tests/scripts/test_gen_shou_circle.rb`
Expected: `LoadError: cannot load such file .../scripts/gen_shou_circle`

- [ ] **Step 1.3: Write the generator**

```ruby
#!/usr/bin/env ruby
# frozen_string_literal: true

# Generate the 「照」spotlight circle background for demo07 as a C header.
# Concentric bands around the screen centre; band N is painted with BG
# palette index N so the whole light show is driven by set_colour() at
# runtime — no pixel math on the Z80 (see the 照 theme design doc).
#
# Usage: ruby scripts/gen_shou_circle.rb demo/shou_circle.h

module ShouCircle
  WIDTH = 256
  HEIGHT = 224          # full BG plane (32x28 tiles) so vertical scroll wraps into dark filler
  VISIBLE_HEIGHT = 192
  CENTRE_X = 128
  CENTRE_Y = 96
  RADII = [14, 20, 26, 32, 38, 44, 50, 56, 62, 68, 74, 80].freeze
  MAX_TILES = 440       # BG tiles use VRAM 0..443; the 4 sprite tiles sit at 444..447
  DARK = 0x00
  WHITE = 0x3F

  def self.band_index(x, y)
    d2 = (x - CENTRE_X)**2 + (y - CENTRE_Y)**2
    RADII.each_with_index do |r, i|
      return i + 1 if d2 <= r * r
    end
    0
  end

  # 64 palette indices for the 8x8 tile at tile coords (tx, ty).
  # Rows below the visible screen are dark filler (band 0).
  def self.tile_at(tx, ty)
    (0...8).flat_map do |row|
      y = ty * 8 + row
      (0...8).map do |col|
        y >= VISIBLE_HEIGHT ? 0 : band_index(tx * 8 + col, y)
      end
    end
  end

  def self.hflip(tile)
    tile.each_slice(8).flat_map(&:reverse)
  end

  def self.vflip(tile)
    tile.each_slice(8).to_a.reverse.flatten
  end

  # Walk the 32x28 plane, dedupe tiles including H/V-flipped variants.
  # Returns [unique_tiles, map_words] where map words carry SMS flip bits.
  def self.build_map
    unique = []
    lookup = {}
    words = []
    (0...(HEIGHT / 8)).each do |ty|
      (0...(WIDTH / 8)).each do |tx|
        tile = tile_at(tx, ty)
        unless lookup.key?(tile)
          idx = unique.length
          unique << tile
          lookup[tile] = [idx, 0x0000]
          h = hflip(tile)
          v = vflip(tile)
          hv = vflip(h)
          lookup[h]  = [idx, 0x0200] unless lookup.key?(h)
          lookup[v]  = [idx, 0x0400] unless lookup.key?(v)
          lookup[hv] = [idx, 0x0600] unless lookup.key?(hv)
        end
        idx, flags = lookup[tile]
        words << (idx | flags)
      end
    end
    [unique, words]
  end

  # 64 indices -> 32 bytes of SMS 4bpp planar data
  # (same layout as scripts/png_to_sms_pattern.rb)
  def self.encode_tile(tile)
    bytes = []
    (0...8).each do |row|
      bp = [0, 0, 0, 0]
      (0...8).each do |col|
        px = tile[row * 8 + col]
        (0...4).each { |plane| bp[plane] |= ((px >> plane) & 1) << (7 - col) }
      end
      bytes.concat(bp)
    end
    bytes
  end

  # Phase-1 palette: dark everywhere, centre disc (band 1) lit white.
  def self.initial_palette
    pal = Array.new(16, DARK)
    pal[1] = WHITE
    pal
  end

  def self.generate(out_path)
    unique, words = build_map
    raise "tile budget blown: #{unique.length} > #{MAX_TILES}" if unique.length > MAX_TILES

    tile_bytes = unique.flat_map { |t| encode_tile(t) }
    map_bytes = words.flat_map { |w| [w & 0xFF, (w >> 8) & 0xFF] }

    File.open(out_path, 'w') do |f|
      f.puts '// Auto-generated by scripts/gen_shou_circle.rb — do not edit'
      f.puts "#define SHOU_CIRCLE_TILE_COUNT #{unique.length}"
      f.puts "#define SHOU_CIRCLE_BAND_COUNT #{RADII.length}"
      f.puts
      f.puts 'const unsigned char shou_circle_palette[] = {'
      f.puts '  ' + initial_palette.map { |b| '0x%02x' % b }.join(', ')
      f.puts '};'
      f.puts
      f.puts 'const unsigned char shou_circle_tiles[] = {'
      tile_bytes.each_slice(16) { |s| f.puts '  ' + s.map { |b| '0x%02x' % b }.join(', ') + ',' }
      f.puts '};'
      f.puts
      f.puts 'const unsigned char shou_circle_map[] = {'
      map_bytes.each_slice(16) { |s| f.puts '  ' + s.map { |b| '0x%02x' % b }.join(', ') + ',' }
      f.puts '};'
    end

    puts "Generated #{out_path}: #{unique.length} tiles, #{tile_bytes.length} tile bytes, #{map_bytes.length} map bytes"
  end
end

if __FILE__ == $0
  abort "Usage: ruby #{$0} <output.h>" unless ARGV[0]
  ShouCircle.generate(ARGV[0])
end
```

- [ ] **Step 1.4: Run tests, verify they pass**

Run: `ruby tests/scripts/test_gen_shou_circle.rb`
Expected: all assertions pass, `0 failures, 0 errors`. Note the unique-tile count printed by a quick manual run (`ruby scripts/gen_shou_circle.rb /tmp/shou_circle_check.h`) — expect roughly 330 tiles. If `test_tile_budget` fails, first check the flip-dedup logic; only then consider coarser outer rings. Never shrink the inner radii (the disc is the spotlight) and never raise `MAX_TILES` past 440 (VRAM 444–447 is reserved for the sprite).

- [ ] **Step 1.5: Commit**

```bash
git add scripts/gen_shou_circle.rb tests/scripts/test_gen_shou_circle.rb
git commit -m "Add circle background generator for demo07"
```

---

### Task 2: demo07 ROM — static whiteout (design phases 2–3)

**Files:**
- Create: `demo/demo07.c`, `demo/demo07.rb`
- Modify: `Makefile` (ZCC `-t` flag, `DEMOS` list, demo07 targets)

- [ ] **Step 2.1: Make Docker builds work in non-TTY shells**

In `Makefile` line 2, change

```make
ZCC = docker run --platform linux/amd64 --rm -i -t -v $(PWD):/src z88dk/z88dk zcc
```

to

```make
ZCC = docker run --platform linux/amd64 --rm -i -v $(PWD):/src z88dk/z88dk zcc
```

(`-t` allocates a pseudo-TTY, which fails under make from agents/CI; it adds nothing for a compiler run.)

Verify: `make demo05` completes and `build/demo05.sms` exists.

- [ ] **Step 2.2: Write `demo/demo07.rb` (whiteout state machine only)**

```ruby
# demo07: 「照」 spotlight → whiteout (phases 2-3: manual band lighting)
# A = light the next band outward; B = darken one band back.
# Band b is BG palette index b. lit == whiteout means index 0 (the
# surround) is white too -> 100% white screen.
prev_btns = 0xFF
lit = 1        # centre disc (band 1) starts white
whiteout = 13  # SHOU_CIRCLE_BAND_COUNT + 1

while true
  btns = read_buttons

  if btns & 0x10 == 0 && prev_btns & 0x10 != 0 # A just pushed
    if whiteout > lit
      lit += 1
      if lit == whiteout
        set_colour(0, 0x3F)
      else
        set_colour(lit, 0x3F)
      end
    end
  end

  if btns & 0x20 == 0 && prev_btns & 0x20 != 0 # B just pushed
    if lit > 1
      if lit == whiteout
        set_colour(0, 0x00)
      else
        set_colour(lit, 0x00)
      end
      lit -= 1
    end
  end

  prev_btns = btns
  wait_vblank
end
```

- [ ] **Step 2.3: Write `demo/demo07.c`**

Modeled on `demo/demo05.c`. `pal1`/`pal2` must exist because `mrubyz.c` externs them; point them at sane palettes even though demo07's Ruby never calls the builtin that loads them.

```c
#include <SMSlib.h>
#include <sms.h>
#include <stdio.h>
#include "mrubyz.h"
#include "shou_circle.h"

long _heap;

const unsigned char sprite_pal[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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

  SMS_VRAMmemset(0x0000, 0x00, 16384);
  SMS_loadTiles(shou_circle_tiles, 0, SHOU_CIRCLE_TILE_COUNT * 32);
  SMS_loadTileMapArea(0, 0, shou_circle_map, 32, 28);
  SMS_loadBGPalette(shou_circle_palette);
  SMS_loadSpritePalette(sprite_pal);
  SMS_useFirstHalfTilesforSprites(0);

  SMS_displayOn();
  SMS_setBackdropColor(0);

  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, demo07_bytecode);
}
```

Note: demo07 loads no text font (the circle tiles occupy that VRAM); the demo prints nothing, and VM error messages would render as garbage tiles — acceptable for a standalone showpiece ROM.

- [ ] **Step 2.4: Wire up the Makefile**

In the root `Makefile`:
- Change `DEMOS = demo01 demo02 demo03 demo04 demo05 demo06` to append ` demo07`.
- Add after the demo06 rules:

```make
demo/shou_circle.h: scripts/gen_shou_circle.rb
	ruby scripts/gen_shou_circle.rb $@

demo07: demo/demo07.c mrubyz.c demo/demo07.ruby.c demo/shou_circle.h | $(BUILDDIR)
	$(ZCC) $(CFLAGS) demo/demo07.c mrubyz.c demo/demo07.ruby.c -o $(BUILDDIR)/demo07 -create-app

demo/demo07.ruby.c: demo/demo07.rb
	$(MRBC) -B demo07_bytecode --verbose -o $@ $<
```

- Append `demo/demo07.ruby.c` to the `BYTECODE_FILES` list (line 19) so `make clean` removes it, matching the existing demos.

(`demo/shou_circle.h` is generated output — do not commit it; the existing `.gitignore` policy for generated files applies, check `git status` stays clean of it or add it to `.gitignore`.)

- [ ] **Step 2.5: Build**

Run: `make demo07`
Expected: `build/demo07.sms` exists, no compile errors. If the linker complains about `timer`, add `volatile uint16_t timer = 0;` to `demo/demo07.c` (mrubyz.c externs it for the `system_timer` builtin).

- [ ] **Step 2.6: Human visual checkpoint (emulator)**

Ask the human to run `build/demo07.sms` in their SMS emulator and confirm:
- Boot: dark screen with a small white disc dead centre (band 1 is pre-lit).
- Each A press lights one more ring outward; the 11th press fills the circle and the 12th turns the whole screen white.
- B presses walk it back down through the disc to full blackout (the spec's 暗転, for act transitions); A from blackout relights the disc.

- [ ] **Step 2.7: Run the VM test suite, then commit**

Run: `make test` — expected: all pass (nothing in mrubyz.c changed, this catches accidents).

```bash
git add demo/demo07.c demo/demo07.rb Makefile
git commit -m "Add demo07 with manual band lighting and whiteout"
```

---

### Task 3: 「照」 sprite (placeholder art + pipeline)

**Files:**
- Create: `scripts/gen_shou_sprite.rb`
- Test: `tests/scripts/test_gen_shou_sprite.rb`
- Modify: `demo/demo07.c`, `demo/demo07.rb`, `Makefile`

The design doc wants hand-drawn 16×16 art (scaled misaki looks crushed). This task builds the *pipeline* with a 2×-scaled misaki 照 as an explicitly-labeled placeholder, so the hand-drawn art later only needs to replace the bitmap source. Strokes use sprite palette index 1 (opaque black), NOT transparent index 0, so the kanji stays black on top of the white disc.

- [ ] **Step 3.1: Write the failing tests**

```ruby
# tests/scripts/test_gen_shou_sprite.rb
require 'minitest/autorun'
require_relative '../../scripts/gen_shou_sprite'

class TestScale2x < Minitest::Test
  def test_doubles_a_single_row
    # 8-bit row 0b10000001 -> 16-bit 0b1100000000000011, as two rows
    out = ShouSprite.scale2x([0x81] + [0x00] * 7)
    assert_equal 16, out.length
    assert_equal 0b1100000000000011, out[0]
    assert_equal 0b1100000000000011, out[1]
    assert_equal 0, out[2]
  end
end

class TestSpriteTiles < Minitest::Test
  def test_splits_into_4_tiles_tl_tr_bl_br
    rows = Array.new(16, 0)
    rows[0]  = 0b1000000000000000  # TL top-left pixel
    rows[15] = 0b0000000000000001  # BR bottom-right pixel
    tl, tr, bl, br = ShouSprite.split_tiles(rows)
    assert_equal 1, tl[0]        # 64-entry index arrays, stroke = index 1
    assert_equal 0, tr[0]
    assert_equal 1, br[63]
    assert_equal 0, bl[63]
  end

  def test_planar_encoding_uses_index1
    tile = Array.new(64, 1)
    bytes = ShouSprite.encode_tile(tile)
    assert_equal 32, bytes.length
    assert_equal [0xFF, 0x00, 0x00, 0x00], bytes[0, 4]
  end
end
```

- [ ] **Step 3.2: Run tests, verify they fail** (`LoadError`)

Run: `ruby tests/scripts/test_gen_shou_sprite.rb`

- [ ] **Step 3.3: Write the generator**

```ruby
#!/usr/bin/env ruby
# frozen_string_literal: true

# Generate the 16x16 「照」 sprite tiles for demo07.
#
# PLACEHOLDER ART: currently scales the 8x8 misaki glyph 2x. The design doc
# calls for hand-drawn 16x16 art (scaled misaki strokes crush together);
# when that art exists, replace bitmap_rows16 with it — the tile
# splitting/encoding below stays the same.
#
# Strokes are sprite palette index 1 (opaque black), not transparent 0,
# so the kanji reads black over the white spotlight disc.
#
# Usage: ruby scripts/gen_shou_sprite.rb misaki_gothic.bdf demo/shou_sprite.h

require_relative 'font_builder'

module ShouSprite
  SHOU = 0x7167 # 照

  # [8 bytes] 8x8 -> [16 x 16-bit rows], each pixel doubled in both axes
  def self.scale2x(rows8)
    rows8.flat_map do |byte|
      wide = 0
      (0...8).each do |bit|
        next unless (byte >> (7 - bit)) & 1 == 1
        wide |= 0b11 << (14 - bit * 2)
      end
      [wide, wide]
    end
  end

  # [16 x 16-bit rows] -> four 64-entry index arrays (TL, TR, BL, BR),
  # set pixels become sprite palette index 1
  def self.split_tiles(rows16)
    quads = [[0, 0], [0, 8], [8, 0], [8, 8]] # [row_off, col_off]
    quads.map do |row_off, col_off|
      (0...8).flat_map do |r|
        row = rows16[row_off + r]
        (0...8).map { |c| (row >> (15 - (col_off + c))) & 1 }
      end
    end
  end

  # Same 4bpp planar layout as scripts/gen_shou_circle.rb
  def self.encode_tile(tile)
    bytes = []
    (0...8).each do |row|
      bp = [0, 0, 0, 0]
      (0...8).each do |col|
        px = tile[row * 8 + col]
        (0...4).each { |plane| bp[plane] |= ((px >> plane) & 1) << (7 - col) }
      end
      bytes.concat(bp)
    end
    bytes
  end

  def self.generate(bdf_path, out_path)
    glyphs = FontBuilder.parse_bdf(File.read(bdf_path))
    glyph = glyphs[SHOU] or abort "no 照 (U+7167) glyph in #{bdf_path}"
    tiles = split_tiles(scale2x(glyph))
    bytes = tiles.flat_map { |t| encode_tile(t) }

    File.open(out_path, 'w') do |f|
      f.puts '// Auto-generated by scripts/gen_shou_sprite.rb — do not edit'
      f.puts '// Tile order: TL, TR, BL, BR (VRAM 444..447 -> put_sprite tiles 188..191)'
      f.puts 'const unsigned char shou_sprite_tiles[] = {'
      bytes.each_slice(16) { |s| f.puts '  ' + s.map { |b| '0x%02x' % b }.join(', ') + ',' }
      f.puts '};'
    end
    puts "Generated #{out_path} (#{bytes.length} bytes)"
  end
end

if __FILE__ == $0
  abort "Usage: ruby #{$0} <misaki.bdf> <output.h>" unless ARGV[1]
  ShouSprite.generate(ARGV[0], ARGV[1])
end
```

- [ ] **Step 3.4: Run tests, verify they pass**

Run: `ruby tests/scripts/test_gen_shou_sprite.rb`

- [ ] **Step 3.5: Load the sprite in `demo/demo07.c` and draw it from Ruby**

In `demo/demo07.c`: add `#include "shou_sprite.h"`, set `sprite_pal[1]` to `0x00` explicitly commented as the stroke colour, and after `SMS_useFirstHalfTilesforSprites(0);` add:

```c
  SMS_loadTiles(shou_sprite_tiles, 444, 128);
```

(444, not 256: the circle background uses BG tiles 0–443, so the sprite occupies the last 4 tiles of the sprite half 256–447. `put_sprite`'s tile argument is relative to 256, hence 188–191 below.)

In `demo/demo07.rb`, add fixed-position sprite drawing to the main loop (movement comes in Task 4). Before the `while true` loop add `shou_x = 120` and `shou_y = 88`; inside the loop, before `btns = read_buttons`:

```ruby
  init_sprites
  put_sprite(shou_x, shou_y, 188)
  put_sprite(shou_x + 8, shou_y, 189)
  put_sprite(shou_x, shou_y + 8, 190)
  put_sprite(shou_x + 8, shou_y + 8, 191)
```

and change the loop tail to:

```ruby
  prev_btns = btns
  wait_vblank
  render_sprites
```

The spec says the kanji must *melt into the white* on full whiteout (「照」の字も白に溶ける), but its stroke is opaque black sprite-palette index 1 — CRAM entry 17. So in both whiteout branches, next to the existing `set_colour(0, ...)` calls, add the matching stroke calls:

```ruby
        set_colour(0, 0x3F)
        set_colour(17, 0x3F) # melt the kanji stroke into the white
```
```ruby
        set_colour(0, 0x00)
        set_colour(17, 0x00)
```

`demo/shou_sprite.h` is generated output like `demo/shou_circle.h` — do not commit it (add to `.gitignore` if needed).

In `Makefile`: add the header rule and make demo07 depend on it:

```make
demo/shou_sprite.h: scripts/gen_shou_sprite.rb scripts/font_builder.rb $(MISAKI_BDF)
	ruby scripts/gen_shou_sprite.rb $(MISAKI_BDF) $@
```

and extend the demo07 target's prerequisites with `demo/shou_sprite.h`.

- [ ] **Step 3.6: Build + human visual checkpoint**

Run: `make demo07`, human runs the ROM: a black 「照」 sits centred on the white disc (readable even if chunky — it's placeholder art), band lighting still works, and on full whiteout the kanji melts into the white (the `set_colour(17, ...)` calls added in Step 3.5), reappearing when B backs off the whiteout.

- [ ] **Step 3.7: Run `make test`, commit**

```bash
git add scripts/gen_shou_sprite.rb tests/scripts/test_gen_shou_sprite.rb demo/demo07.c demo/demo07.rb Makefile
git commit -m "Add (placeholder) shou sprite to demo07"
```

---

### Task 4: Spotlight tracking (design phase 1)

**Files:**
- Modify: `demo/demo07.rb`

The 照 sprite follows the D-pad instantly; the background circle chases it lazily via `set_scroll`, a few pixels per frame, adds/compares only. Scroll targets are clamped to ±32 px (X) / ±16 px (Y) so the wrapped plane edge never shows (see geometry note at top).

- [ ] **Step 4.1: Add movement + lag-follow to `demo/demo07.rb`**

Complete final file:

```ruby
# demo07: 「照」 spotlight → whiteout
# D-pad: move 照 (the white disc chases it)
# A: light the next band outward   B: darken one band back (down to full blackout)
prev_btns = 0xFF
lit = 1        # centre disc (band 1) starts white
whiteout = 13  # SHOU_CIRCLE_BAND_COUNT + 1

shou_x = 120   # sprite top-left; centre of the 16x16 is (shou_x+8, shou_y+8)
shou_y = 88
ox = 0         # current circle-centre offset from screen centre, in px
oy = 0

while true
  init_sprites
  put_sprite(shou_x, shou_y, 188)
  put_sprite(shou_x + 8, shou_y, 189)
  put_sprite(shou_x, shou_y + 8, 190)
  put_sprite(shou_x + 8, shou_y + 8, 191)

  btns = read_buttons

  if btns & 0x01 == 0 # UP held
    shou_y -= 2 if shou_y > 16
  end
  if btns & 0x02 == 0 # DOWN held
    shou_y += 2 if 160 > shou_y
  end
  if btns & 0x04 == 0 # LEFT held
    shou_x -= 2 if shou_x > 16
  end
  if btns & 0x08 == 0 # RIGHT held
    shou_x += 2 if 224 > shou_x
  end

  # circle chases 照: move the scroll offset 2px/frame toward the target
  target_ox = shou_x + 8 - 128
  target_oy = shou_y + 8 - 96
  target_ox = 32 if target_ox > 32
  target_ox = -32 if -32 > target_ox
  target_oy = 16 if target_oy > 16
  target_oy = -16 if -16 > target_oy

  if target_ox > ox
    ox += 2
  elsif ox > target_ox
    ox -= 2
  end
  if target_oy > oy
    oy += 2
  elsif oy > target_oy
    oy -= 2
  end

  # +X scroll shifts the image right; +Y scroll shifts it up (so negate oy)
  sy = 0 - oy
  sy += 224 if 0 > sy # 224 = BG plane height in px; VDP Y scroll wraps at 224

  if btns & 0x10 == 0 && prev_btns & 0x10 != 0 # A just pushed
    if whiteout > lit
      lit += 1
      if lit == whiteout
        set_colour(0, 0x3F)
        set_colour(17, 0x3F) # melt the kanji stroke into the white
        set_colour(16, 0x3F) # CRAM 16 = backdrop/border
      else
        set_colour(lit, 0x3F)
      end
    end
  end

  if btns & 0x20 == 0 && prev_btns & 0x20 != 0 # B just pushed
    if lit > 0 # reversal reaches lit == 0: full blackout, per the spec's 暗転まで
      if lit == whiteout
        set_colour(0, 0x00)
        set_colour(17, 0x00)
        set_colour(16, 0x00)
      else
        set_colour(lit, 0x00)
      end
      lit -= 1
    end
  end

  prev_btns = btns
  wait_vblank
  set_scroll(ox & 0xFF, sy) # write the scroll registers inside vblank
  render_sprites
end
```

Notes for the implementer:
- All position variables stay even (start even, move by 2), so `ox`/`oy` land exactly on their targets without oscillating.
- No `<` anywhere — the VM doesn't implement it. Comparisons are written with `>` only.
- `ox & 0xFF` maps negative X offsets onto the SMS's mod-256 X scroll; `sy` is kept in 0..223 by the `+ 224` wrap, which is why `set_scroll`'s Y argument never sees an out-of-range value.

- [ ] **Step 4.2: Build**

Run: `make demo07` — expect a clean build (this is Ruby-only, but mrbc can reject syntax and the VM has no `<`, so a build catches compile errors early).

- [ ] **Step 4.3: Human visual checkpoint (emulator) — includes scroll-direction verification**

The scroll sign convention (`+X right`, `+Y up`) is asserted from VDP behavior but **must be verified on screen**: move 照 right — the disc should drift right after it. If the disc runs the *opposite* way, negate the final values (`set_scroll((0 - ox) & 0xFF, ...)` / drop the `0 -` on `oy`) — one-line fix, then re-verify. Also confirm:
- Disc trails the kanji smoothly and settles centred under it.
- Pushing 照 to the play-area edges never shows a plane seam (dark wrap regions only).
- Band lighting + whiteout still behave; during whiteout the kanji melts to white.

- [ ] **Step 4.4: Run `make test`, commit**

```bash
git add demo/demo07.rb
git commit -m "Add spotlight tracking to demo07"
```

---

### Task 5: Wrap-up

- [ ] **Step 5.1: Full verification**

Run: `make clean && make && make test`
Expected: all demo ROMs (01–07) build, VM test suite passes.

- [ ] **Step 5.2: Note the follow-ups**

Report (do not implement): remaining 照-theme work per the design doc — hand-drawn 16×16 「照」 art to replace the placeholder, the demo05 "Flash Invader" rework, optional flat palette fade for demo04 page turns, and the deferred decision about folding demo07 into the presentation ROM.

---

## Execution record (2026-07-15)

Executed subagent-driven on main; all tasks complete, per-task spec + quality reviews plus a final whole-implementation review. Commits: 15d1ee6 (Task 1 generator + tests), 6967373 (review fix: restore plan radii, MAX_TILES 440 with sprite tiles moved to VRAM 444–447), 37e6333 (review fix: flip-semantics regression tests), 88ec89f (Task 2 ROM + Makefile -t fix), 6f23ca3 (Task 3 sprite pipeline + hygiene fixes), 5e3ac7c (Task 4 spotlight tracking), 00f9cac (review fix: Integer#& VM test — the ox & 0xFF idiom depends on two's-complement AND), f33dd3a (review fix: scroll write moved inside vblank), and 2d4c95c (final-review fix: B-reversal reaches full blackout per the spec's 暗転まで; border CRAM 16 whitened on whiteout; Makefile BDF warning updated). Plan listings above were kept in sync with the delivered code. Known accepted deviations: demo07.sms is 49152 bytes without a TMR SEGA header (same as demo06; bank the asset data like demo04's title.c if US-hardware support is ever needed); sprite art is the documented misaki-2× placeholder. Emulator visual verification (scroll sign, melt, seams, blackout) is the remaining human step.
