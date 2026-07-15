require 'minitest/autorun'
require 'tmpdir'
require_relative '../../scripts/gen_shou_text'

class TestMessage < Minitest::Test
  def test_message_is_the_18_char_reveal
    # trailing space: its reveal press rings the carriage bell on its own,
    # separated from the 9's clack
    assert_equal 'KANSAI RUBYKAIGI9 ', ShouText::MESSAGE
    assert_equal 18, ShouText::MESSAGE.length
  end
end

class TestTextTileEncoding < Minitest::Test
  def test_stroke_pixels_set_all_four_planes
    # stroke colour is BG palette index 15 (0b1111), so a glyph row byte
    # appears verbatim in every bitplane
    bytes = ShouText.encode_tile([0x81] + [0x00] * 7)
    assert_equal 32, bytes.length
    assert_equal [0x81, 0x81, 0x81, 0x81], bytes[0, 4]
    assert_equal [0x00, 0x00, 0x00, 0x00], bytes[4, 4]
  end

  def test_space_becomes_a_blank_tile
    assert_equal [0x00] * 32, ShouText.encode_tile([0x00] * 8)
  end
end

class TestGenerateText < Minitest::Test
  def test_generate_writes_tiles_and_defs_companion
    font = Array.new(256 * 8, 0)
    font['K'.ord * 8] = 0xFF # give K a recognisable top row
    Dir.mktmpdir do |dir|
      bin = File.join(dir, 'FONT8.BIN')
      File.binwrite(bin, font.pack('C*'))
      path = File.join(dir, 'shou_text.h')
      capture_io { ShouText.generate(bin, path) }

      content = File.read(path)
      assert_includes content, 'shou_text_tiles'
      assert_equal 18 * 32, content.scan(/0x[0-9a-f]{2}/).length
      assert_includes content, '0xff', 'K top row should appear in the data'

      defs = File.read(File.join(dir, 'shou_text_defs.h'))
      assert_includes defs, '#define SHOU_TEXT_LEN 18'
      refute_includes defs, 'shou_text_tiles', 'defs header must not define the array'
    end
  end
end
