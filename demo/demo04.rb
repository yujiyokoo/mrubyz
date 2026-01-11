pres_data = [
"\r\r\r   Minimal presentation\r\r     - Yuji Yokoo, 2026\r",
"\r\r\r   Now I can write my\r   presentation in Ruby\r",
"\r\r\r   Watch this space\r    for further\r     development!\r",
"\r\r\r   Thank you!!!\r",
]

idx = 0
prev_btns = 0xFF
while pres_data.size > idx
  btns = read_buttons
  if btns & 0x10 == 0 && prev_btns & 0x10 != 0
    clear_screen
    gotoxy(0, 0)
    puts pres_data[idx]
    idx += 1
  end
  prev_btns = btns
end
