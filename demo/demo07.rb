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
shown = 0      # letters of "KANSAI RUBYKAIGI9" revealed during whiteout

while true
  btns = read_buttons

  moved = false

  if btns & 0x01 == 0 # UP held
    if shou_y > 16
      shou_y -= 2
      moved = true
    end
  end

  if btns & 0x02 == 0 # DOWN held
    if 160 > shou_y
      shou_y += 2
      moved = true
    end
  end

  if btns & 0x04 == 0 # LEFT held
    if shou_x > 16
      shou_x -= 2
      moved = true
    end
  end

  if btns & 0x08 == 0 # RIGHT held
    if 224 > shou_x
      shou_x += 2
      moved = true
    end
  end

  # piko-piko footsteps at maximum cadence while 照 is actually walking
  # (and visible — silent during the whiteout). start_footstep rate-limits
  # itself: it retriggers as soon as the previous blip finishes, giving one
  # step every 2 loop frames.
  if moved && whiteout > lit
    start_footstep
  end

  # draw 照 at its moved position, so sprite and scroll agree on the frame
  # (init_sprites must run every frame: it resets the sprite list,
  #  otherwise put_sprite keeps appending until the 64-entry SAT is full)
  init_sprites

  # tiles 188..191 = VRAM 444..447, see shou_sprite.h
  put_sprite(shou_x, shou_y, 188)
  put_sprite(shou_x + 8, shou_y, 189)
  put_sprite(shou_x, shou_y + 8, 190)
  put_sprite(shou_x + 8, shou_y + 8, 191)

  # circle chases 照: 2px/frame while more than 8px behind (keeping up with
  # 照's own 2px/frame), easing to a 1px/frame glide for the last stretch.
  # Frozen during whiteout so the message stays centred on screen.
  if whiteout > lit
    target_ox = shou_x + 8 - 128
    target_oy = shou_y + 8 - 96

    # the light may roam only while its lit bands stay on the BG plane edge-free:
    # lit radius = 8 + 6*lit px, so the safe scroll range shrinks as it grows.
    # With just the disc lit (lit == 1) this allows the whole screen; unlit
    # bands wrapping around the plane edge are black-on-black and invisible.
    cx = 119 - 6 * lit
    cy = 87 - 6 * lit
    target_ox = cx if target_ox > cx
    target_ox = 0 - cx if 0 - cx > target_ox
    target_oy = cy if target_oy > cy
    target_oy = 0 - cy if 0 - cy > target_oy

    if target_ox > ox
      if target_ox - ox > 8
        ox += 2
      else
        ox += 1
      end
    elsif ox > target_ox
      if ox - target_ox > 8
        ox -= 2
      else
        ox -= 1
      end
    end

    if target_oy > oy
      if target_oy - oy > 8
        oy += 2
      else
        oy += 1
      end
    elsif oy > target_oy
      if oy - target_oy > 8
        oy -= 2
      else
        oy -= 1
      end
    end
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
        ox = 0 # snap the (invisible) light home so the message centres on screen
        oy = 0
      else
        set_colour(lit, 0x3F)
      end
    elsif 17 > shown # fully white: reveal the message letter by letter
      reveal_letter(shown)
      shown += 1
    end
  end

  if btns & 0x20 == 0 && prev_btns & 0x20 != 0 # B just pushed
    if lit > 0 # lit 1 -> 0 darkens the centre disc: full blackout
      if lit == whiteout
        hide_letters if shown > 0 # restore the circle map under the message
        shown = 0
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
  sfx_update
  wait_vblank
  set_scroll(ox & 0xFF, sy)
  render_sprites
end
