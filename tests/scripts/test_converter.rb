require 'minitest/autorun'

# Stub ENCODING_MAP for tests (don't load the real generated file)
ENCODING_MAP = {
  " " => 1, "H" => 2, "e" => 3, "l" => 4, "o" => 5,
  "W" => 6, "r" => 7, "d" => 8, "!" => 9,
  "あ" => 10, "い" => 11,
  "光" => 0x1BB # a 9-bit code, exercises the high bit
}.freeze

require_relative '../../scripts/converter'

class TestCommandDetection < Minitest::Test
  def test_colour1_is_command
    assert Converter.command_string?("-colour1-\r")
  end

  def test_colour0_is_command
    assert Converter.command_string?("-colour0-\r")
  end

  def test_plain_text_is_not_command
    refute Converter.command_string?("Hello world\r")
  end

  def test_japanese_text_is_not_command
    refute Converter.command_string?("あいう\r")
  end
end

class TestCodeEncoding < Minitest::Test
  def test_encodes_ascii_to_codes
    assert_equal [2, 3, 4, 4, 5, 0x00D], Converter.encode_codes("Hello\r")
  end

  def test_encodes_highlight_marker
    assert_equal [0x001, 2, 0x001], Converter.encode_codes("{hl}H{hl}")
  end

  def test_encodes_japanese
    assert_equal [10, 0x00D], Converter.encode_codes("あ\r")
  end

  def test_high_codes_pass_through
    assert_equal [0x1BB], Converter.encode_codes("光")
  end

  def test_aborts_on_missing_char
    assert_raises(SystemExit) { Converter.encode_codes("あZ\r") }
  end
end

class TestPacking < Minitest::Test
  def test_one_full_block
    codes = [0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48]
    packed = Converter.pack_blocks(codes)
    assert_equal 9, packed.bytesize
    assert_equal 0x00, packed.bytes[0]                    # no high bits
    assert_equal [0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48], packed.bytes[1, 8]
  end

  def test_mask_bit_i_is_char_i_high_bit
    codes = [0x1BB, 0x41, 0x100, 0x42, 0x43, 0x44, 0x45, 0x46]
    packed = Converter.pack_blocks(codes)
    assert_equal 0b00000101, packed.bytes[0]              # chars 0 and 2 have bit 8
    assert_equal 0xBB, packed.bytes[1]
    assert_equal 0x00, packed.bytes[3]                    # low byte of 0x100
  end

  def test_pads_final_block_with_pad_code
    packed = Converter.pack_blocks([0x41, 0x42])
    assert_equal 9, packed.bytesize
    # PAD = 0x1FF: high bit set for chars 2..7 -> mask 0b11111100
    assert_equal 0b11111100, packed.bytes[0]
    assert_equal [0x41, 0x42, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF], packed.bytes[1, 8]
  end

  def test_multi_block
    codes = Array.new(9, 0x41)                            # 9 chars -> 2 blocks
    packed = Converter.pack_blocks(codes)
    assert_equal 18, packed.bytesize
    assert_equal 0b11111110, packed.bytes[9]              # 7 PADs in block 2
  end

  def test_pack_string_composes_encode_and_pack
    packed = Converter.pack_string("Hello\r")
    assert_equal 9, packed.bytesize
    assert_equal [2, 3, 4, 4, 5, 0x0D], packed.bytes[1, 6]
    assert_equal 0b11000000, packed.bytes[0]              # 2 PADs
  end

  def test_packed_output_is_binary_encoding
    assert_equal Encoding::BINARY, Converter.pack_string("H").encoding
  end
end

class TestStringFormatter < Minitest::Test
  def test_formats_as_hex_escapes
    str = "\x01\x02\x0D"
    result = Converter.format_string(str)
    assert_equal '"\\x01\\x02\\x0D"', result
  end

  def test_empty_string
    assert_equal '""', Converter.format_string("")
  end
end
