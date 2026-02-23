# Title screen
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

# Main presentation
i = 0
prev_btns = 0xFF
while true
  j = 0
  clear_screen
  gotoxy(0, 0)
  txt_col(0) # change the colour back in case it's changed

  # draw first paragraph
  puts pres_data[i][j] if pres_data[i].size > j
  j += 1

  going_back = false
  # + 1 since we want to wait for keypress after last sentence
  while pres_data[i].size + 1 > j
    btns = read_buttons
    # A button
    if btns & 0x10 == 0 && prev_btns & 0x10 != 0
      # Don't try to display if we've gone past the end
      # We just wanted to wait for the keypress

      # handle special commands
      if pres_data[i].size > j && pres_data[i][j].eql?("-colour1-\r")
        txt_col(1)
        j += 1
      elsif pres_data[i].size > j && pres_data[i][j].eql?("-colour0-\r")
        txt_col(0)
        j += 1
      end

      puts pres_data[i][j] if pres_data[i].size > j
      j += 1
    end

    # B button
    if btns & 0x20 == 0 && prev_btns & 0x20 != 0
      j = pres_data[i].size + 1 # instead of break
      i -= 1 unless i == 0
      going_back = true
    end
    prev_btns = btns
  end

  # go forward, unless going back (B pressed)
  i += 1 unless going_back

  # don't go past the last page, so we can go back in navigation
  i -= 1 if pres_data.size == i
end
