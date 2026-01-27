pres_data =
[["\r\r   mrubyz, a minimal mruby VM\r\r      for Zilog Z80\r\r\r     - Yuji Yokoo, 2026\r\r"],
 ["  Who is this?\r",
  "\r  Yuji Yokoo\r    based in Tokyo, Japan\r\r  Fediverse\r    @yujiyokoo@ruby.social\r\r  Programmer\r\r"],
 ["  What is this project about???\r",
  "\r\r   Running mruby bytecode\r\r      on Zilog Z80\r\r"],
 ["  What is mruby?\r",
  "\r  * smaller, embeddable\r    Ruby implementation\r",
  "\r  * suitable for small\r    environments\r\r"],
 ["  What is Ruby VM\r",
  "\r  * Virtual Machine\r",
  "\r  * When Ruby is run, the\r    source is compiled into\r    bytecode first\r",
  "\r  * The VM runs the bytecode\r\r"],
 ["  What is bytecode?\r",
  "\r  * VM instructions\r",
  "\r  * Simple instructions\r\r"],
 ["  Bytecode example\r\r  Ruby:\r",
  "\r    return 2 == 2\r",
  "\r  Bytecode:\r",
  "\r  1 000 LOADI_2    R1\r  1 002 LOADI_2    R2\r  1 004 EQ         R1    R2\r  1 006 RETURN     R1\r  1 008 STOP\r\r"],
 ["  What is mruby/c\r",
  "\r  * Smaller mruby, VM-only\r",
  "\r  * /c for compact, or\r    concurrent\r",
  "\r  * Got inspiration for\r    mrubyz\r\r"],
 ["  What is Z80?\r",
  "\r  * Popular processor from the\r    80s\r",
  "\r  * Primarily 8-bit\r",
  "\r  * Used for many platforms\r\r    * Sega Master System\r    * Sega Game Gear\r    * Sinclair ZX Spectrum\r    * Many more\r\r"],
 ["  What is Master System?\r",
  "\r  * Sega's Z80 console\r",
  "\r  * Z80 at 3.5MHz\r",
  "\r  * 8KB RAM\r    64KB address space\r    32KB ROM without bank switch\r",
  "\r  * 16KB VRAM, 256x192px\r\r\r"],
 ["  What is a minimal VM?\r",
  "\r  A program that reads and\r\r  executes the bytecode\r\r  (pruced by the official\r     mruby compiler, mrbc)\r\r"],
 ["  Why write your own?\r",
  "\r  I wanted to run mruby code\r",
  "\r  Couldn't build mruby/c\r",
  "\r  No gcc or clang.\r\r  zcc frontend with sdcc or sccz80\r",
  "\r  Rather than starting with a big\r  project with possibly many bugs\r  I can introduce, start small\r",
  "  with fewer bugs and features\r\r"],
 ["  Is there prior art on 8bit?\r",
  "\r    Yes\r",
  "\r micro ruby\r https://github.com/\r  kishima/micro_mruby_for_arduino_uno\r",
  "\r nesruby\r https://github.com/yhara/nesruby\r",
  "\r ruby-gameboy\r https://github.com/\r  khasinski/ruby-gameboy\r\r"],
 ["  What is supported?\r",
  "\r",
  "  * built-in functions\r",
  "  * Assignment\r",
  "  * Add, sub, multi, div\r",
  "  * Branching with if\r",
  "  * while loop\r",
  "  * equality\r",
  "  * greater than (>)\r\r"],
 ["  What is not supported?\r",
  "  * most things\r",
  "  * less than (<), etc.\r",
  "  * string manipulation\r",
  "  * exception\r",
  "  * class\r",
  "  * proc\r",
  "  * function definitions\r",
  "  * each\r\r"],
 ["  What can be written?\r",
  "\r  This presentation tool!\r",
  "\r  j = 0\r  while pages.size > j\r    if btn_a_pressed?\r      puts pages[j]\r    end\r  end\r\r"],
 ["  What's the dev process like?\r",
  "\r  * use mrbc to generate bytecode\r",
  "\r  * load it and run\r",
  "\r  * find unsupported insturcion\r",
  "\r  * add support, or give up\r\r"],
 ["  Limitation sand workaround\r\r"],
 ["  How is the performance?\r",
  "\r    Absolutely not great\r",
  "\r    but not entirely unusable\r\r"],
 ["  Demo!\r"]]

i = 0
prev_btns = 0xFF
while true
  j = 0
  clear_screen
  gotoxy(0, 0)

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
