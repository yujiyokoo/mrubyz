require 'minitest/autorun'
require 'tmpdir'
require_relative '../../scripts/gen_shou_sprite'

class TestShouBitmap < Minitest::Test
  def test_is_a_16x16_bitmap
    assert_equal 16, ShouSprite::SHOU_BITMAP.length
    ShouSprite::SHOU_BITMAP.each { |row| assert_operator row, :<=, 0xFFFF }
  end

  def test_matches_the_jiskan16_glyph
    # spot-check rows of the JIS 0x3E48 glyph as extracted from jiskan16.bdf
    assert_equal 0x0000, ShouSprite::SHOU_BITMAP[0]
    assert_equal 0x7dfe, ShouSprite::SHOU_BITMAP[1]  # top strokes of 日 and 召
    assert_equal 0x6666, ShouSprite::SHOU_BITMAP[14] # the 灬 dots
    assert_equal 0xc223, ShouSprite::SHOU_BITMAP[15]
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

class TestGenerateSprite < Minitest::Test
  def test_generate_writes_4_tiles_from_the_baked_bitmap
    Dir.mktmpdir do |dir|
      path = File.join(dir, 'shou_sprite.h')
      capture_io { ShouSprite.generate(path) }
      content = File.read(path)
      assert_includes content, 'shou_sprite_tiles'
      assert_equal 4 * 32, content.scan(/0x[0-9a-f]{2}/).length
    end
  end
end
