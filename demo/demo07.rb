# demo07: 「照」 spotlight → whiteout (phases 2-3: manual band lighting)
# A = light the next band outward; B = darken one band back.
# Band b is BG palette index b. lit == whiteout means index 0 (the
# surround) is white too -> 100% white screen.
prev_btns = 0xFF
lit = 1        # centre disc (band 1) starts white
whiteout = 13  # SHOU_CIRCLE_BAND_COUNT + 1

while true
  btns = read_buttons

  if btns & 0x10 == 0 && prev_btns & 0x10 != 0 # A just pushed
    if whiteout > lit
      lit += 1
      if lit == whiteout
        set_colour(0, 0x3F)
      else
        set_colour(lit, 0x3F)
      end
    end
  end

  if btns & 0x20 == 0 && prev_btns & 0x20 != 0 # B just pushed
    if lit > 1
      if lit == whiteout
        set_colour(0, 0x00)
      else
        set_colour(lit, 0x00)
      end
      lit -= 1
    end
  end

  prev_btns = btns
  wait_vblank
end
