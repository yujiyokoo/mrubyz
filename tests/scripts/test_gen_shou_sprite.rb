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
