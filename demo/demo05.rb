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
  bullet_type = 0
  enemy_bullet_x = nil
  enemy_bullet_y = nil

  clear_screen
  set_background
  player_hit = enemy_hit = false

  while !player_hit && !enemy_hit
    init_sprites

    btns = read_buttons

    if btns & 0x10 == 0 && prev_btns & 0x10 != 0 # A just pushed
      if !bullet_x && !bullet_y
        bullet_x = player_x
        bullet_y = 21
        bullet_type = 0
        start_sfx2
      end
    end

    if btns & 0x20 == 0 && prev_btns & 0x20 != 0 # B just pushed
      if !bullet_x && !bullet_y
        bullet_x = player_x
        bullet_y = 21
        bullet_type = 1
        start_sfx2
      end
    end

    if bullet_x && bullet_y
      if bullet_y == enemy_y && (bullet_x == enemy_x || (bullet_type == 1 && (enemy_x == bullet_x - 1 || enemy_x == bullet_x + 1)))
        put_sprite(enemy_x * 8, enemy_y * 8, 4)
        enemy_hit = true
      elsif bullet_y > -1
        put_sprite(bullet_x * 8, bullet_y * 8, 3)
        if bullet_type == 1
          put_sprite((bullet_x - 1) * 8, bullet_y * 8, 3)
          put_sprite((bullet_x + 1) * 8, bullet_y * 8, 3)
        end
      end
    end

    if enemy_bullet_x && enemy_bullet_y
      if enemy_bullet_y == 22 && (enemy_bullet_x == player_x)
        put_sprite(player_x * 8, 22 * 8, 4)
        player_hit = true
      elsif 24 > enemy_bullet_y
        put_sprite(enemy_bullet_x * 8, enemy_bullet_y * 8, 3)
      end
    else
      enemy_bullet_x = enemy_x
      enemy_bullet_y = enemy_y + 1
      start_sfx1
    end

    put_sprite(player_x * 8, 22 * 8, 1) unless player_hit
    put_sprite(enemy_x * 8, enemy_y * 8, 2) unless enemy_hit

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
      enemy_x += enemy_direction if !player_hit && !enemy_hit
      enemy_wait = 3
    end

    if bullet_x && bullet_y
      if bullet_y == -1
        bullet_x = bullet_y = nil
      else
        bullet_y -= 1
      end
    end

    if enemy_bullet_x && enemy_bullet_y
      if enemy_bullet_y == 24
        enemy_bullet_x = enemy_bullet_y = nil
      else
        enemy_bullet_y += 1
      end
    end

    sfx_update
    wait_vblank
    render_sprites
    bg_scroll
  end

  play_end_sfx
  wait_vblank
  bg_reset

  gotoxy(7, 11)
  puts "Press A to restart"

  while player_hit || enemy_hit
    btns = read_buttons
    if btns & 0x10 == 0 && prev_btns & 0x10 != 0
      player_hit = enemy_hit = false
    end
    prev_btns = btns
    player_x = 15
    wait_vblank
  end
end
