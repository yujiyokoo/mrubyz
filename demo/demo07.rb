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
  # tiles 188..191 = VRAM 444..447, see shou_sprite.h
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
        set_colour(16, 0x3F) # CRAM 16 = backdrop/border
        set_colour(17, 0x3F) # melt the kanji stroke into the white
      else
        set_colour(lit, 0x3F)
      end
    end
  end

  if btns & 0x20 == 0 && prev_btns & 0x20 != 0 # B just pushed
    if lit > 0 # lit 1 -> 0 darkens the centre disc: full blackout
      if lit == whiteout
        set_colour(0, 0x00)
        set_colour(16, 0x00) # CRAM 16 = backdrop/border
        set_colour(17, 0x00)
      else
        set_colour(lit, 0x00)
      end
      lit -= 1
    end
  end

  prev_btns = btns
  wait_vblank
  set_scroll(ox & 0xFF, sy)
  render_sprites
end
