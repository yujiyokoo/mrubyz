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
