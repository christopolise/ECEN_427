#include "alien_march.h"
#include "sprites.h"
#include "hdmi.h"
#include "bullet.h"
#include <stdint.h>
#include <stdbool.h>
#include "globals.h"

#define ALIENS_NUM_ROWS 5
#define ALIENS_NUM_COLS 11
#define SCALE 2
#define ERROR -1
#define ALIEN_START_X 20
#define ALIEN_START_Y 50
#define ALIEN_DISTANCE 30
#define ALIEN_SPRITE_LEN 12
#define ALIEN_SPRITE_HEIGHT 8
#define ALIEN_ERASER_LEN 3
#define ALIEN_STEP_SIZE 3
#define EXPLOSION_HEIGHT 10
#define EXPLOSION_ERASER_LEN 16
#define ERASER_OFFSET SCALE * 2
#define ALIEN_DOWN_STEP 4
#define ROW0 0
#define ROW1 1
#define ROW2 2
#define ROW3 3
#define ROW4 4
#define ALIEN1_SCORE 10
#define ALIEN2_SCORE 20
#define ALIEN3_SCORE 40
#define ALIEN_MARCH_SPEED 15
#define SCREEN_BUF_RIGHT 20
#define SCREEN_BUF_LEFT 50
#define COLOR_ARRAY_SIZE 3
#define BULLET_OFFSET 10
#define BUNKER_Y_POS 350
#define BULLET_WAIT 10


static char fore[COLOR_ARRAY_SIZE] = {0xff, 0xff, 0xff};
static char back[COLOR_ARRAY_SIZE] = {0x00, 0x00, 0x00};

static int8_t alien_to_erase_x = ERROR;
static int8_t alien_to_erase_y = ERROR;

struct alien_t {
  uint16_t pos_x;
  uint16_t pos_y;
  bool is_alive;
};

struct aliens_t {
  struct alien_t aliens[ALIENS_NUM_COLS][ALIENS_NUM_ROWS];
  bool are_going_right;
};
static struct aliens_t aliens;

// initialize all of the alien fields
void alien_march_init() {
  aliens.are_going_right = true;
  // for each row
  for (uint16_t row = 0; row < ALIENS_NUM_ROWS; ++row) {
    // for each column
    for (uint16_t col = 0; col < ALIENS_NUM_COLS; ++col) {
      aliens.aliens[col][row].pos_x = col * ALIEN_DISTANCE + ALIEN_START_X;
      aliens.aliens[col][row].pos_y = row * ALIEN_DISTANCE + ALIEN_START_Y;
      aliens.aliens[col][row].is_alive = true;
    }
  }
}

// prints out all of the aliens to HDMI
void print_aliens() {
  static bool out = true;
  // for each row
  for (uint16_t row = 0; row < ALIENS_NUM_ROWS; ++row) {
    // for each column
    for (uint16_t col = 0; col < ALIENS_NUM_COLS; ++col) {
      // if the alien is alive
      if (aliens.aliens[col][row].is_alive) {
        // toggle between in and out sprites
        if (out) {
          // different rows have different sprites
          if (row == ROW0) {
            hdmi_draw_sprite(sprite_alien_top_out_12x8, ALIEN_SPRITE_LEN, ALIEN_SPRITE_HEIGHT, SCALE, fore, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x);
          }
          else if (row < ROW3) {
            hdmi_draw_sprite(sprite_alien_middle_out_12x8, ALIEN_ERASER_LEN, ALIEN_SPRITE_HEIGHT, SCALE, back, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x - 4);
            hdmi_draw_sprite(sprite_alien_middle_out_12x8, ALIEN_ERASER_LEN, ALIEN_SPRITE_HEIGHT, SCALE, back, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x + 23);
            hdmi_draw_sprite(sprite_alien_middle_out_12x8, ALIEN_SPRITE_LEN, ALIEN_SPRITE_HEIGHT, SCALE, fore, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x);
          }
          else {
            hdmi_draw_sprite(sprite_alien_bottom_out_12x8, ALIEN_ERASER_LEN, ALIEN_SPRITE_HEIGHT, SCALE, back, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x - 4);
            hdmi_draw_sprite(sprite_alien_bottom_out_12x8, ALIEN_ERASER_LEN, ALIEN_SPRITE_HEIGHT, SCALE, back, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x + 23);
            hdmi_draw_sprite(sprite_alien_bottom_out_12x8, ALIEN_SPRITE_LEN, ALIEN_SPRITE_HEIGHT, SCALE, fore, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x);
          }
        }
        else {
          // different rows have different sprites
          if (row == ROW0) {
            hdmi_draw_sprite(sprite_alien_top_in_12x8, ALIEN_SPRITE_LEN, ALIEN_SPRITE_HEIGHT, SCALE, fore, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x);
          }
          else if (row < ROW3) {
            hdmi_draw_sprite(sprite_alien_middle_out_12x8, ALIEN_ERASER_LEN, ALIEN_SPRITE_HEIGHT, SCALE, back, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x - 4);
            hdmi_draw_sprite(sprite_alien_middle_out_12x8, ALIEN_ERASER_LEN, ALIEN_SPRITE_HEIGHT, SCALE, back, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x + 23);
            hdmi_draw_sprite(sprite_alien_middle_in_12x8, ALIEN_SPRITE_LEN, ALIEN_SPRITE_HEIGHT, SCALE, fore, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x);
          }
          else {
            hdmi_draw_sprite(sprite_alien_bottom_out_12x8, ALIEN_ERASER_LEN, ALIEN_SPRITE_HEIGHT, SCALE, back, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x - 4);
            hdmi_draw_sprite(sprite_alien_bottom_out_12x8, ALIEN_ERASER_LEN, ALIEN_SPRITE_HEIGHT, SCALE, back, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x + 23);
            hdmi_draw_sprite(sprite_alien_bottom_in_12x8, ALIEN_SPRITE_LEN, ALIEN_SPRITE_HEIGHT, SCALE, fore, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x);
          }
        }
      }
    }
  }
  out = !out;
}

// checks to see whether or not an alien has collided with a bullet
// @param[in] bullet_x the x position of the bullet
// @param[in] bullet_y the y position of the bullet
// @return true if the bullet collided with an alien; false if otherwise
bool alien_collision(uint16_t bullet_x, uint16_t bullet_y) {
  // for each row
  for (uint16_t row = 0; row < ALIENS_NUM_ROWS; ++row) {
    // for each column
    for (uint16_t col = 0; col < ALIENS_NUM_COLS; ++col) {
      // if the alien is alive
      if (aliens.aliens[col][row].is_alive) {
        uint16_t alien_x_min = aliens.aliens[col][row].pos_x;
        uint16_t alien_y_min = aliens.aliens[col][row].pos_y;
        uint16_t alien_x_max = aliens.aliens[col][row].pos_x + ALIEN_SPRITE_LEN * SCALE;
        uint16_t alien_y_max = aliens.aliens[col][row].pos_y + ALIEN_SPRITE_HEIGHT * SCALE;

        // if we hit an alien
        if (bullet_x >= alien_x_min && bullet_x <= alien_x_max && bullet_y >= alien_y_min && bullet_y <= alien_y_max) {
          hdmi_draw_sprite(sprite_alien_bottom_in_12x8, ALIEN_SPRITE_LEN, ALIEN_SPRITE_HEIGHT, SCALE, back, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x);
          hdmi_draw_sprite(sprite_alien_explosion_12x10, ALIEN_SPRITE_LEN, ALIEN_SPRITE_HEIGHT, SCALE, fore, back, aliens.aliens[col][row].pos_y, aliens.aliens[col][row].pos_x);
          aliens.aliens[col][row].is_alive = false;
          alien_to_erase_x = col;
          alien_to_erase_y = row;
          // different rows of aliens get different scores
          if (row == ROW0) {
            globals_setScore(globals_getScore() + ALIEN3_SCORE);
          }
          if (row == ROW1 || row == ROW2) {
            globals_setScore(globals_getScore() + ALIEN2_SCORE);
          }
          if (row == ROW3 || row == ROW4) {
            globals_setScore(globals_getScore() + ALIEN1_SCORE);
          }
          return true;
        }
      }
    }
  }

  return false;
}

// determines whether there are any aliens left
// @return true if there are, false if not
bool alien_march_all_dead() {
  bool all_dead = true;
  // for each row
  for (uint16_t row = 0; row < ALIENS_NUM_ROWS; ++row) {
    // for each column
    for (uint16_t col = 0; col < ALIENS_NUM_COLS; ++col) {
      all_dead &= !aliens.aliens[col][row].is_alive;
    }
  }

  return all_dead;
}

// tick function for the marching aliens
void alien_march_tick() {
  static uint8_t cnt = 0U;
  ++cnt;
  // slowly move the aliens
  if (cnt == ALIEN_MARCH_SPEED) {
    cnt = 0;
    uint8_t y_change = 0U;

    // edge of screen detection
    // for each row
    for (uint16_t row = 0; row < ALIENS_NUM_ROWS; ++row) {
      // for each column
      for (uint16_t col = 0; col < ALIENS_NUM_COLS; ++col) {
        // if the alien is at the edge of the screen
        if ((aliens.aliens[col][row].pos_x >= HDMI_DISPLAY_WIDTH - SCREEN_BUF_LEFT) || 
            (aliens.aliens[col][row].pos_x < SCREEN_BUF_RIGHT)) {
          y_change = ALIEN_DOWN_STEP;
          aliens.are_going_right = !aliens.are_going_right;
          break;
        }
      }
    }

    //move all aliens
    // for each row
    for (uint16_t row = 0; row < ALIENS_NUM_ROWS; ++row) {
      // for each column
      for (uint16_t col = 0; col < ALIENS_NUM_COLS; ++col) {
        // if we've reached the edge of the screen
        if (y_change) {
          hdmi_draw_sprite(sprite_alien_top_out_12x8, ALIEN_SPRITE_LEN, 2, SCALE,
                    back,
                    back,
                    aliens.aliens[col][row].pos_y,
                    aliens.aliens[col][row].pos_x);

          // if the alien is alive and has reached the bunkers
          if (aliens.aliens[col][row].is_alive && aliens.aliens[col][row].pos_y > BUNKER_Y_POS) {
            globals_set_game_over();
            return;
          }
        }
        // check the direction the aliens are currently going
        if (aliens.are_going_right) {
          aliens.aliens[col][row].pos_x += ALIEN_STEP_SIZE;
        }
        else {
          aliens.aliens[col][row].pos_x -= ALIEN_STEP_SIZE;
        }
        aliens.aliens[col][row].pos_y += y_change;
      }
    }

    // erase dead aliens
    if (alien_to_erase_x != ERROR && alien_to_erase_y != ERROR) {
      hdmi_draw_sprite(sprite_alien_explosion_12x10,
                       EXPLOSION_ERASER_LEN,
                       EXPLOSION_HEIGHT,
                       SCALE,
                       back,
                       back,
                       aliens.aliens[alien_to_erase_x][alien_to_erase_y].pos_y,
                       aliens.aliens[alien_to_erase_x][alien_to_erase_y].pos_x - ERASER_OFFSET);
    }

    print_aliens();

    // check if all are dead
    if (alien_march_all_dead()) {
      alien_march_init();
      globals_setLives(globals_getLives() + 1);
    }
  }

  // hit detection
  uint16_t bullet_x;
  uint16_t bullet_y;
  bullet_get_player_bullet_coordinates(&bullet_x, &bullet_y);
  //if we have a collision
  if (alien_collision(bullet_x, bullet_y)) {
    bullet_set_player_bullet_collide();
  }

  //fire bullets
  if (cnt == BULLET_WAIT) {
    // see which bullets are inactive
    for (uint8_t bullet = 0; bullet < ALIEN_BULLETS; ++bullet) {
      // if a given bullet is inactive
      if (!bullet_is_alien_bullet_active(bullet)) {
        uint8_t alien_x = rand() % ALIENS_NUM_COLS; //pick a random alien x
        int8_t alien_y;
        bool found = false;
        for(alien_y = 4; alien_y >= 0; --alien_y) {
          // if we found a live alien
          if (aliens.aliens[alien_x][alien_y].is_alive) {
            found = true;
            break;
          }
        }
        // if we didn't find a live alien for that column
        if (!found) {
          continue;
        }

        uint16_t bullet_x = aliens.aliens[alien_x][alien_y].pos_x;
        uint16_t bullet_y = aliens.aliens[alien_x][alien_y].pos_y;
        bullet_alien_fire(bullet, bullet_x + BULLET_OFFSET, bullet_y + BULLET_OFFSET);
        break;
      }
    }
  }
}
