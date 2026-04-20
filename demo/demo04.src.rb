prev_btns = 0xFF
waiting = true
show_title_page

while waiting
  btns = read_buttons
  # Check A button
  if btns & 0x10 == 0 && prev_btns & 0x10 != 0
    waiting = false
  end
  prev_btns = btns
end

clear_screen
restore_font

def render_page_progress(idx, total)
  total -= 1 if total > 1 # adjust for 0-based index
  x = 31 * 8 * idx / total
  put_sprite x, 23 * 8, 191
end

def render_time_progress(start_time)
  secs_per_tile = 1500 / 31 # 25 min
  timer_s = system_timer
  # NOTE: This only supports 2 laps of the timer (16 bit)

  x = (timer_s - start_time) / secs_per_tile
  x = 31 if x > 31

  put_sprite x * 8, 23 * 8, 190
end

# Main presentation
i = 0
prev_btns = 0xFF
start_time = system_timer
while true
  j = 0
  init_sprites

  render_page_progress(i, pres_data.size)
  render_time_progress(start_time)
  gotoxy(0, 0)

  wait_vblank
  clear_screen
  txt_col(0) # change the colour back in case it's changed
  render_sprites

  # draw first paragraph
  puts pres_data[i][j] if pres_data[i].size > j
  j += 1

  current_col = 0
  # + 1 since we want to wait for keypress after last sentence
  while pres_data[i].size + 1 > j
    btns = read_buttons
    # A button
    if btns & 0x10 == 0 && prev_btns & 0x10 != 0
      # Don't try to display if we've gone past the end
      # We just wanted to wait for the keypress
      if pres_data[i].size > j
        if pres_data[i][j] == "VMRUN\r"
          print "        "
          print vmrun
        else
          puts pres_data[i][j]
        end
      end
      j += 1
    end

    # If Up pressed
    if btns & 0x01 == 0
      if current_col != 1
        txt_col(1)
        current_col = 1
      end
    else
      if current_col != 0
        txt_col(0)
        current_col = 0
      end
    end

    # B button
    if btns & 0x20 == 0 && prev_btns & 0x20 != 0
      j = pres_data[i].size + 1 # instead of break
      i -= 1 unless i == 0
      j = 0
			clear_screen
      if pres_data[i].size > j
        if pres_data[i][j] == "VMRUN\r"
          print "        "
          print vmrun
        else
          puts pres_data[i][j]
        end
      end
			j += 1
    end
    prev_btns = btns
  end

  # go forward, unless going back (B pressed)
  i += 1

  # don't go past the last page, so we can go back in navigation
  i -= 1 if pres_data.size == i
end
