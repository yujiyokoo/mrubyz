require 'minitest/autorun'

# Stub ENCODING_MAP for tests (don't load the real generated file)
ENCODING_MAP = {
  " " => 1, "H" => 2, "e" => 3, "l" => 4, "o" => 5,
  "W" => 6, "r" => 7, "d" => 8, "!" => 9,
  "あ" => 10, "い" => 11
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

class TestStringEncoding < Minitest::Test
  def test_encodes_ascii
    result = Converter.encode_string("Hello\r")
    assert_equal [2, 3, 4, 4, 5, 0x0D], result.bytes
  end

  def test_passes_through_cr
    result = Converter.encode_string("H\r")
    assert_equal [2, 0x0D], result.bytes
  end

  def test_encodes_japanese
    result = Converter.encode_string("あ\r")
    assert_equal [10, 0x0D], result.bytes
  end

  def test_aborts_on_missing_char
    assert_raises(SystemExit) { Converter.encode_string("あZ\r") }
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
