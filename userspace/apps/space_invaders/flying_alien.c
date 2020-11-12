#include "flying_alien.h"
#include "bullet.h"
#include "globals.h"
#include "hdmi.h"
#include "sounds.h"
#include "sprites.h"

#define SCALE 2
#define COLOR_ARRAY_SIZE 3
#define Y_COORD 30
#define SAUCER_LEN 18
#define SAUCER_HEIGHT 7
#define CNT_INIT 401
#define CNT1 300
#define CNT2 400
#define RIGHT_OFFSET 50
#define LEFT_OFFSET 30
#define SAUCER_STEP 2
#define SAUCER_BONUS (50 + (rand() % 5) * 50)

static char magenta[COLOR_ARRAY_SIZE] = {0xff, 0x00, 0xff};
static char back[COLOR_ARRAY_SIZE] = {0x00, 0x00, 0x00};

static bool is_alive = true;
static uint16_t alien_pos_x = LEFT_OFFSET;

// checks if a bullet hit the saucer
// @return true if it did, false if not
bool saucer_collision(uint16_t bullet_x, uint16_t bullet_y) {
  uint16_t alien_x_min = alien_pos_x;
  uint16_t alien_y_min = Y_COORD;
  uint16_t alien_x_max = alien_pos_x + SAUCER_LEN * SCALE;
  uint16_t alien_y_max = Y_COORD + SAUCER_HEIGHT * SCALE;

  // if we hit the saucer
  if (bullet_x >= alien_x_min && bullet_x <= alien_x_max &&
      bullet_y >= alien_y_min && bullet_y <= alien_y_max) {
    hdmi_draw_sprite(sprite_saucer_18x7, SAUCER_LEN, SAUCER_HEIGHT, SCALE, back,
                     back, Y_COORD, alien_pos_x);
    is_alive = false;
    return true;
  }

  return false;
}

// standard tick function
void flying_alien_tick() {
  static bool is_going_right = true;
  static bool is_gone = false;
  static uint16_t cnt = CNT_INIT;

  // reset the is_alive flag
  if (cnt == CNT1) {
    is_alive = true;
  }
  // count if bleow CNT2
  if (cnt < CNT2) {
    ++cnt;
  } else if (cnt == CNT2) {
    is_gone = false;
  }

  // check x position
  if (alien_pos_x > HDMI_DISPLAY_WIDTH - RIGHT_OFFSET) {
    alien_pos_x = HDMI_DISPLAY_WIDTH - RIGHT_OFFSET;
    sounds_toggle_looping(false);
    is_gone = true;
    is_going_right = false;
    cnt = 0;
    hdmi_draw_sprite(sprite_saucer_18x7, SAUCER_LEN, SAUCER_HEIGHT, SCALE, back,
                     back, Y_COORD, alien_pos_x);
  } else if (alien_pos_x < LEFT_OFFSET) {
    alien_pos_x = LEFT_OFFSET;
    sounds_toggle_looping(false);
    is_gone = true;
    is_going_right = true;
    cnt = 0;
    hdmi_draw_sprite(sprite_saucer_18x7, SAUCER_LEN, SAUCER_HEIGHT, SCALE, back,
                     back, Y_COORD, alien_pos_x);
  }
  // if the saucer is still on the screen
  if (!is_gone) {
    if (!globals_isBulletPlayed() && !globals_isExplosionPlayed() &&
        sounds_is_available()) {
      sounds_toggle_looping(true);
      sounds_play(SOUNDS_UFO_INDX);
    } else {
      sounds_toggle_looping(false);
    }
    // check the direction of the saucer
    if (is_going_right) {
      alien_pos_x += SAUCER_STEP;
    } else {
      alien_pos_x -= SAUCER_STEP;
    }
    // draw it if it's alive
    if (is_alive) {
      hdmi_draw_sprite(sprite_saucer_18x7, SAUCER_LEN, SAUCER_HEIGHT, SCALE,
                       magenta, back, Y_COORD, alien_pos_x);
    }
  }

  // check collisions
  uint16_t bullet_x;
  uint16_t bullet_y;

  bullet_get_player_bullet_coordinates(&bullet_x, &bullet_y);
  // if we hit the saucer
  if (saucer_collision(bullet_x, bullet_y)) {
    bullet_set_player_bullet_collide();
    globals_setScore(globals_getScore() + SAUCER_BONUS);
    sounds_toggle_looping(false);
    sounds_play(SOUNDS_UFO_DIE_INDX);
  }
}
