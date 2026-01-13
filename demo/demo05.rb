idx = 0
prev_btns = 0xFF

while true
  player_x = 15
  enemy_x = 7
  enemy_y = 2
  enemy_direction = 1
  enemy_wait = 3
  bullet_x = nil
  bullet_y = nil

  clear_screen
  game_on = true

  while game_on
    gotoxy(player_x-1, 22) # -1 for the left space (' A ')
    print ' A '
    gotoxy(enemy_x-1, enemy_y)
    print ' V '
    btns = read_buttons

    # check btns & 0x20 == 0 if you wanna check for B
    if btns & 0x10 == 0 && prev_btns & 0x10 != 0 # A just pushed
      if !bullet_x && !bullet_y
        bullet_x = player_x
        bullet_y = 21
      end
    end

    if bullet_x && bullet_y
      if bullet_x == enemy_x && bullet_y == enemy_y
        gotoxy(bullet_x, bullet_y)
        print '*'
        game_on = false
      elsif bullet_y > -1
        gotoxy(bullet_x, bullet_y)
        print '|'
      end
      unless bullet_y == 21
        gotoxy(bullet_x, bullet_y+1)
        print ' '
      end
    end

    prev_btns = btns

    if btns & 0x04 == 0 # LEFT held
      player_x -= 1 unless player_x == 1
    end

    if btns & 0x08 == 0 # RIGHT held
      player_x += 1 unless player_x == 30
    end

    if enemy_x == 1 || enemy_x == 30
      if enemy_direction == 1
        enemy_direction = -1
      else
        enemy_direction = 1
      end
    end

    enemy_wait -= 1
    if enemy_wait == 0
      enemy_x += enemy_direction
      enemy_wait = 3
    end

    if bullet_x && bullet_y
      if bullet_y == -1
        bullet_x = bullet_y = nil
      else
        bullet_y -= 1
      end
    end

    wait_vblank
  end

  gotoxy(7, 11)
  puts "Press A to restart"

  while !game_on
    btns = read_buttons
    if btns & 0x10 == 0 && prev_btns & 0x10 != 0
      game_on = true
    end
    prev_btns = btns
    player_x = 15
    wait_vblank
  end
end
