require 'minitest/autorun'
require_relative '../../scripts/font_builder'

BDF_FIXTURE = <<~BDF
  FONT_ASCENT 6
  FONT_DESCENT 2
  STARTCHAR space
  ENCODING 32
  BBX 8 8 0 -2
  BITMAP
  00
  00
  00
  00
  00
  00
  00
  00
  ENDCHAR
  STARTCHAR A
  ENCODING 65
  BBX 8 8 0 -2
  BITMAP
  18
  24
  42
  7E
  42
  42
  00
  00
  ENDCHAR
  STARTCHAR uni3042
  ENCODING 12354
  BBX 8 8 0 -2
  BITMAP
  00
  10
  7C
  D6
  54
  38
  10
  00
  ENDCHAR
BDF

class TestBdfParser < Minitest::Test
  def setup
    @glyphs = FontBuilder.parse_bdf(BDF_FIXTURE)
  end

  def test_parses_correct_number_of_glyphs
    assert_equal 3, @glyphs.size
  end

  def test_parses_space_codepoint
    assert @glyphs.key?(32), "expected codepoint 32 (space)"
    assert_equal [0,0,0,0,0,0,0,0], @glyphs[32]
  end

  def test_parses_ascii_A
    assert @glyphs.key?(65)
    assert_equal [0x18,0x24,0x42,0x7E,0x42,0x42,0x00,0x00], @glyphs[65]
  end

  def test_parses_hiragana_a
    assert @glyphs.key?(12354)
    assert_equal [0x00,0x10,0x7C,0xD6,0x54,0x38,0x10,0x00], @glyphs[12354]
  end

  def test_positions_glyph_at_top_when_bbx_fills_from_ascent
    # BBX 8 5 0 1: yoff=1, height=5, FONT_ASCENT=6 → top_tile_row = 6-1-5 = 0
    # Glyph fills rows 0-4; rows 5-7 blank
    short_bdf = <<~BDF
      FONT_ASCENT 6
      STARTCHAR short
      ENCODING 90
      BBX 8 5 0 1
      BITMAP
      FF
      00
      FF
      00
      FF
      ENDCHAR
    BDF
    glyphs = FontBuilder.parse_bdf(short_bdf)
    assert_equal 8, glyphs[90].size
    assert_equal [0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00], glyphs[90]
  end

  def test_clips_rows_outside_tile_bounds
    # BBX 8 10 0 0: yoff=0, height=10, FONT_ASCENT=6 → top_tile_row = -4
    # Bitmap rows 0-3 are above the tile (clipped); rows 4-9 map to tile rows 0-5;
    # tile rows 6-7 are beyond the bitmap → blank
    long_bdf = <<~BDF
      FONT_ASCENT 6
      STARTCHAR long
      ENCODING 91
      BBX 8 10 0 0
      BITMAP
      01
      02
      03
      04
      05
      06
      07
      08
      09
      0A
      ENDCHAR
    BDF
    glyphs = FontBuilder.parse_bdf(long_bdf)
    assert_equal 8, glyphs[91].size
    assert_equal [0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x00, 0x00], glyphs[91]
  end

  def test_bbx_vertical_offset_adds_top_padding
    # BBX 2 2 0 1: height=2, yoff=1, FONT_ASCENT=6 → top_tile_row = 6-1-2 = 3
    # Rows 0-2 blank; rows 3-4 contain glyph; rows 5-7 blank
    bdf = <<~BDF
      FONT_ASCENT 6
      STARTCHAR dot
      ENCODING 46
      BBX 2 2 0 1
      BITMAP
      C0
      C0
      ENDCHAR
    BDF
    glyphs = FontBuilder.parse_bdf(bdf)
    assert_equal [0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00], glyphs[46]
  end

  def test_bbx_xoff_shifts_pixels_right
    # BBX 2 2 2 1: xoff=2, yoff=1, FONT_ASCENT=6 → top_tile_row=3, pixels shifted right by 2
    # 0xC0 = 1100_0000; >> 2 = 0011_0000 = 0x30
    bdf = <<~BDF
      FONT_ASCENT 6
      STARTCHAR middot
      ENCODING 183
      BBX 2 2 2 1
      BITMAP
      C0
      C0
      ENDCHAR
    BDF
    glyphs = FontBuilder.parse_bdf(bdf)
    assert_equal [0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00], glyphs[183]
  end

  def test_skips_encoding_minus_one
    unmapped_bdf = <<~BDF
      STARTCHAR unmapped
      ENCODING -1
      BBX 8 8 0 0
      BITMAP
      FF
      FF
      FF
      FF
      FF
      FF
      FF
      FF
      ENDCHAR
    BDF
    glyphs = FontBuilder.parse_bdf(unmapped_bdf)
    assert_empty glyphs
    refute glyphs.key?(-1)
    refute glyphs.key?(nil)
  end
end

class TestCharacterScanner < Minitest::Test
  def test_collects_unique_chars
    text = "ab\rcd\rab\r"
    chars = FontBuilder.scan_characters(text)
    assert_equal ['a', 'b', 'c', 'd'].sort, chars.sort
  end

  def test_skips_carriage_return
    chars = FontBuilder.scan_characters("a\rb\r")
    refute_includes chars, "\r"
  end

  def test_skips_command_strings
    text = "-colour1-\r"
    chars = FontBuilder.scan_characters(text)
    assert_empty chars
  end

  def test_handles_japanese
    text = "あいう\r"
    chars = FontBuilder.scan_characters(text)
    assert_includes chars, "あ"
    assert_includes chars, "い"
    assert_includes chars, "う"
  end

  def test_returns_sorted_unique_chars
    text = "bba\r"
    chars = FontBuilder.scan_characters(text)
    assert_equal chars, chars.uniq
    assert_equal chars, chars.sort_by { |c| c.codepoints.first }
  end

  def test_skips_newline_characters
    # Raw file content uses \n — scan_characters must handle it
    chars = FontBuilder.scan_characters("a\nb\r")
    refute_includes chars, "\n"
    assert_includes chars, "a"
    assert_includes chars, "b"
  end

  def test_does_not_skip_text_ending_with_word_pattern
    # "hello-world-\r" should NOT be treated as a command — pattern must be anchored at start
    chars = FontBuilder.scan_characters("hello-world-\r")
    assert_includes chars, "h"
    assert_includes chars, "e"
  end
end

class TestTileIndexAssigner < Minitest::Test
  def test_alphanumerics_at_ascii_positions
    map = FontBuilder.assign_indices(['A', 'b', '3'])
    assert_equal 65, map['A']
    assert_equal 98, map['b']
    assert_equal 51, map['3']
  end

  def test_space_at_ascii_position
    map = FontBuilder.assign_indices([' ', 'あ'])
    assert_equal 32, map[' ']
  end

  def test_always_includes_all_alphanumerics
    map = FontBuilder.assign_indices(['!'])
    FontBuilder::FIXED_CHARS.each do |c|
      assert_equal c.ord, map[c], "expected #{c.inspect} at ASCII position #{c.ord}"
    end
  end

  def test_non_alnum_packed_into_free_slots
    map = FontBuilder.assign_indices(['!', '@', 'あ'])
    # These should be in low free slots (1, 2, 3...) not at ASCII positions
    assert map['!'] < 48, "non-alnum should be packed below alphanumeric range"
    assert map['@'] < 48
  end

  def test_skips_reserved_indices
    chars = ['!', '#', '$', '%', '^', '&', '*', '(', ')', '+', ',', '-']
    map = FontBuilder.assign_indices(chars)
    refute_includes map.values, 0x00
    refute_includes map.values, 0x0C
    refute_includes map.values, 0x0D
  end

  def test_covers_all_input_chars
    chars = ['a', 'b', 'c', 'あ']
    map = FontBuilder.assign_indices(chars)
    chars.each { |c| assert map.key?(c), "expected #{c.inspect} in map" }
  end
end

class TestFontDataGenerator < Minitest::Test
  def setup
    @glyphs = { 32 => [0]*8, 65 => [0x18,0x24,0x42,0x7E,0x42,0x42,0,0] }
    # space => tile 1, A => tile 2
    @map = { " " => 1, "A" => 2 }
    @output = FontBuilder.generate_font_data_h(@map, @glyphs)
  end

  def test_defines_tile_count
    assert_match(/DEMO04_FONT_TILE_COUNT\s+2/, @output)
  end

  def test_contains_c_array
    assert_match(/const uint8_t demo04_font\[\]/, @output)
  end

  def test_space_glyph_all_zeros
    assert_match(/0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/, @output)
  end

  def test_A_glyph_bytes
    assert_match(/0x18, 0x24, 0x42, 0x7E, 0x42, 0x42, 0x00, 0x00/, @output)
  end

  def test_glyph_order_follows_tile_index
    # Extract lines containing "/* tile N */" and verify space (tile 1) comes before A (tile 2)
    tile_lines = @output.lines.select { |l| l.include?("/* tile") }
    tile1_line = tile_lines.find { |l| l.include?("/* tile 1 */") }
    tile2_line = tile_lines.find { |l| l.include?("/* tile 2 */") }

    assert tile1_line, "expected a line for tile 1"
    assert tile2_line, "expected a line for tile 2"

    # tile 1 (space) should be all zeros
    assert_match(/0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/, tile1_line)
    # tile 2 (A) should have the correct glyph
    assert_match(/0x18, 0x24, 0x42, 0x7E, 0x42, 0x42, 0x00, 0x00/, tile2_line)

    # tile 1 appears before tile 2 in the file
    assert @output.index("/* tile 1 */") < @output.index("/* tile 2 */"),
      "tile 1 should appear before tile 2"
  end
end

FONT_BIN_FIXTURE = begin
  data = Array.new(2048, 0)
  # Place a known glyph for 'A' (codepoint 65) at offset 65*8 = 520
  [0x18, 0x24, 0x42, 0x7E, 0x42, 0x42, 0x00, 0x00].each_with_index do |byte, i|
    data[65 * 8 + i] = byte
  end
  data.pack('C*')
end

class TestStandardFontBinParser < Minitest::Test
  def setup
    @glyphs = FontBuilder.parse_standard_font_bin(FONT_BIN_FIXTURE)
  end

  def test_returns_all_256_entries
    assert_equal 256, @glyphs.size
  end

  def test_known_glyph_at_correct_offset
    assert_equal [0x18, 0x24, 0x42, 0x7E, 0x42, 0x42, 0x00, 0x00], @glyphs[65]
  end

  def test_returns_8_bytes_per_glyph
    @glyphs.each_value { |g| assert_equal 8, g.size }
  end

  def test_zero_codepoint_is_blank
    assert_equal [0] * 8, @glyphs[0]
  end
end

class TestEncodingMapGenerator < Minitest::Test
  def setup
    @map = { " " => 1, "A" => 2, "あ" => 3 }
    @output = FontBuilder.generate_encoding_map_rb(@map)
  end

  def test_assigns_constant
    assert_match(/ENCODING_MAP\s*=/, @output)
  end

  def test_contains_space_mapping
    assert_match(/"\ " => 1/, @output)
  end

  def test_contains_ascii_mapping
    assert_match(/"A" => 2/, @output)
  end

  def test_contains_japanese_mapping
    assert_match(/"あ" => 3/, @output)
  end

  def test_is_valid_ruby
    assert_silent { eval(@output) }
    assert_equal 1, ENCODING_MAP[" "]
    assert_equal 3, ENCODING_MAP["あ"]
  end
end
