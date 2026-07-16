#!/usr/bin/env ruby
# Convert an image to SMS tile data, using a small region as a repeating background pattern.
# Tiles that contain the "foreground color" (brightest palette entry) are treated as text and
# kept as unique tiles; all others reuse the tiled pattern from the specified region.
#
# Usage:
#   ruby png_to_sms_pattern.rb <pixels.rgb> <palette.rgb> <out_prefix> \
#     [palette_offset] [tile_offset] [pattern_x] [pattern_y] [pattern_w] [pattern_h] [fg_index]
#
# Defaults: palette_offset=8, tile_offset=0, pattern at (0,160) 32x32, fg_index=auto
# pixels.rgb is raw RGB (3 bytes per pixel). Each pixel is mapped to nearest palette color.
# tile_offset is added to every tilemap entry so it points at the right VRAM tile.

pixels_file, palette_file, out_prefix = ARGV[0], ARGV[1], ARGV[2]
palette_offset   = (ARGV[3] || 8).to_i
tile_offset      = (ARGV[4] || 0).to_i
pattern_x        = (ARGV[5] || 0).to_i
pattern_y        = (ARGV[6] || 160).to_i
pattern_w        = (ARGV[7] || 32).to_i
pattern_h        = (ARGV[8] || 32).to_i
fg_index_arg     = ARGV[9]  # nil means auto

WIDTH, HEIGHT = 256, 192
TILES_X, TILES_Y = WIDTH / 8, HEIGHT / 8

# --- Convert RGB palette to SMS 6-bit format (--BBGGRR), and pick brightest as text color ---
palette_rgb_bytes = File.binread(palette_file).bytes
palette_rgb = palette_rgb_bytes.each_slice(3).to_a  # [[r,g,b], ...]
sms_palette = []
brightnesses = []
# SMS levels in 8-bit space (4 evenly-spaced values per channel)
SMS_LEVELS = [0, 85, 170, 255]

# Pick the nearest SMS color by 3D Euclidean distance — preserves hue
# better than per-channel rounding, which collapses warm tones to gray.
def nearest_sms_color(r, g, b)
  best_dist = Float::INFINITY
  best = [0, 0, 0]
  4.times do |sr|
    4.times do |sg|
      4.times do |sb|
        d = (SMS_LEVELS[sr] - r) ** 2 + (SMS_LEVELS[sg] - g) ** 2 + (SMS_LEVELS[sb] - b) ** 2
        if d < best_dist
          best_dist = d
          best = [sr, sg, sb]
        end
      end
    end
  end
  best
end

palette_rgb.each do |r, g, b|
  sms_r, sms_g, sms_b = nearest_sms_color(r, g, b)
  sms_palette << [sms_r, sms_g, sms_b]  # store as triples for post-processing
  brightnesses << (r + g + b)
end
fg_index = fg_index_arg ? fg_index_arg.to_i : brightnesses.each_with_index.max[1]
fg_pixel = fg_index + palette_offset

# --- Post-process palette for better visual result ---
# 1) Force the foreground (brightest) color to pure white 0x3f
# 2) Warm non-FG colors by bumping R; darken by reducing G/B.
# Tunable via ENV vars RED_SHIFT, DARKEN_R, DARKEN_G, DARKEN_B (defaults: 0, 1, 0, 1)
# Workflow: bump R by RED_SHIFT, then subtract DARKEN_* from each channel.
# Default darkens R and removes blue (which caused purple-ish bricks) while keeping G variation.
RED_SHIFT = (ENV['RED_SHIFT']  || '1').to_i
DARKEN_R  = (ENV['DARKEN_R']   || '1').to_i
DARKEN_G  = (ENV['DARKEN_G']   || '0').to_i
DARKEN_B  = (ENV['DARKEN_B']   || '1').to_i
sms_palette = sms_palette.each_with_index.map do |(sr, sg, sb), i|
  if i == fg_index
    [3, 3, 3]  # pure white
  else
    nr = [[sr + RED_SHIFT - DARKEN_R, 0].max, 3].min
    ng = [sg - DARKEN_G, 0].max
    nb = [sb - DARKEN_B, 0].max
    [nr, ng, nb]
  end
end
sms_palette = sms_palette.map { |sr, sg, sb| (sb << 4) | (sg << 2) | sr }

# --- Read RGB pixels and map each to nearest palette index ---
raw = File.binread(pixels_file).bytes
rgb_to_idx = {}
pixels = Array.new(WIDTH * HEIGHT)
(0...(WIDTH * HEIGHT)).each do |i|
  r, g, b = raw[i * 3], raw[i * 3 + 1], raw[i * 3 + 2]
  key = (r << 16) | (g << 8) | b
  idx = rgb_to_idx[key] ||= palette_rgb.each_with_index.min_by { |(pr, pg, pb), _|
    (pr - r) ** 2 + (pg - g) ** 2 + (pb - b) ** 2
  }[1]
  pixels[i] = idx
end

# Extract an 8x8 tile from absolute (px, py) origin in the indexed image.
def extract_tile(pixels, px, py, palette_offset)
  tile = []
  (0...8).each do |row|
    (0...8).each do |col|
      idx = (py + row) * WIDTH + (px + col)
      tile << (pixels[idx] + palette_offset)
    end
  end
  tile
end

# --- Build the pattern tile set from the specified region ---
pattern_tiles = []
(0...(pattern_h / 8)).each do |ty|
  (0...(pattern_w / 8)).each do |tx|
    pattern_tiles << extract_tile(pixels, pattern_x + tx * 8, pattern_y + ty * 8, palette_offset)
  end
end
pattern_tiles.uniq!
pattern_w_tiles = pattern_w / 8
pattern_h_tiles = pattern_h / 8

# The background colour = the most common pixel in the pattern region.
# Foreground merging keeps EVERY non-background colour (text, badge art,
# mid-tones); the old rule kept only the single brightest colour, which
# silently erased any third colour from unique tiles.
bg_pixel = pattern_tiles.flatten.tally.max_by { |_, count| count }[0]

# --- Walk image tile-by-tile, choose pattern tile or add as unique ---
unique_tiles = pattern_tiles.dup
tile_indices = []
text_tile_count = 0

(0...TILES_Y).each do |ty|
  (0...TILES_X).each do |tx|
    tile = extract_tile(pixels, tx * 8, ty * 8, palette_offset)

    # The pattern tile that would appear at this screen position
    pattern_idx = (ty % pattern_h_tiles) * pattern_w_tiles + (tx % pattern_w_tiles)
    pattern_idx = pattern_idx % pattern_tiles.length
    pattern = pattern_tiles[pattern_idx]

    # Replace only exact-background pixels with the repeating pattern, so
    # the texture stays seamless behind art while all other colours survive
    merged = tile.each_with_index.map { |px, i| px == bg_pixel ? pattern[i] : px }

    if merged == pattern
      # Pure background — reuse the pattern tile
      tile_indices << pattern_idx
    else
      if (existing = unique_tiles.index(merged))
        tile_indices << existing
      else
        tile_indices << unique_tiles.length
        unique_tiles << merged
        text_tile_count += 1
      end
    end
  end
end

# --- Encode each tile to SMS 4bpp planar format ---
tile_bytes = []
unique_tiles.each do |tile|
  (0...8).each do |row|
    bp = [0, 0, 0, 0]
    (0...8).each do |col|
      px = tile[row * 8 + col]
      (0...4).each do |plane|
        bp[plane] |= ((px >> plane) & 1) << (7 - col)
      end
    end
    tile_bytes.concat(bp)
  end
end

# --- Tilemap: 2 bytes per entry, with tile_offset added so map points at VRAM-loaded tiles ---
tilemap_bytes = []
tile_indices.each do |idx|
  vram_idx = idx + tile_offset
  tilemap_bytes << (vram_idx & 0xFF)
  tilemap_bytes << ((vram_idx >> 8) & 0x01)
end

# --- Output as C header ---
File.open("#{out_prefix}.h", "w") do |f|
  f.puts "// Auto-generated from #{pixels_file}"
  f.puts "#define #{out_prefix.upcase}_TILE_COUNT #{unique_tiles.length}"
  f.puts
  # Pad palette to 8 entries (with 0x00) so C code that copies 8 bytes is safe
  padded_palette = sms_palette + [0] * (8 - sms_palette.length)
  f.puts "const unsigned char #{out_prefix}_palette[] = {"
  f.puts "  " + padded_palette.map { |b| "0x%02x" % b }.join(", ")
  f.puts "};"
  f.puts
  f.puts "const unsigned char #{out_prefix}_tiles[] = {"
  tile_bytes.each_slice(16) { |s| f.puts "  " + s.map { |b| "0x%02x" % b }.join(", ") + "," }
  f.puts "};"
  f.puts
  f.puts "const unsigned char #{out_prefix}_map[] = {"
  tilemap_bytes.each_slice(16) { |s| f.puts "  " + s.map { |b| "0x%02x" % b }.join(", ") + "," }
  f.puts "};"
end

puts "Generated #{out_prefix}.h"
puts "  Foreground:    palette index #{fg_index} (brightest)"
puts "  Pattern tiles: #{pattern_tiles.length}"
puts "  Text tiles:    #{text_tile_count}"
puts "  Total tiles:   #{unique_tiles.length}"
puts "  Tile data:     #{tile_bytes.length} bytes"
puts "  Tilemap:       #{tilemap_bytes.length} bytes"
puts "  Palette:       #{sms_palette.length} entries"
