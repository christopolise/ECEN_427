#include "bunkers.h"
#include "bullet.h"
#include "hdmi.h"
#include "sprites.h"

#define SCALE 2
#define BUNKER_LIVES 4U
#define NUM_BUNKERS 4
#define NUM_BUNKER_AREAS 9
#define BUNKER_BLOCK_SIZE 6
#define ELEM0 0
#define ELEM1 1
#define ELEM2 2
#define ELEM3 3
#define ELEM4 4
#define ELEM5 5
#define BUNKER_BLOCK_SPRITE_SIZE 6
#define BUNKER_Y_POS 350
#define BUNKER_1_X_POS 100
#define BUNKER_2_X_POS 225
#define BUNKER_3_X_POS 350
#define BUNKER_4_X_POS 475
#define BUNKER_LEN 24
#define BUNKER_HEIGHT_BLOCKS 3
#define BUNKER_LENGTH_BLOCKS 4
#define BUNKER_HEIGHT 18
#define BUNKER_DISTANCE 125
#define BULLET_OFFSET 10
#define COLOR_ARRAY_SIZE 3
#define MININALLY_DAMAGED 3
#define DAMAGED 2
#define HEAVILY_DAMAGED 1
#define GONE 0
#define BUNKER_ROW0 0
#define BUNKER_ROW1 1
#define BUNKER_ROW2 2
#define BUNKER_COL0 0
#define BUNKER_COL1 1
#define BUNKER_COL2 2
#define BUNKER_COL3 3

static char fore2[COLOR_ARRAY_SIZE] = {0x00, 0xff, 0x00};
static char back[COLOR_ARRAY_SIZE] = {0x00, 0x00, 0x00};

struct bunker_t {
  uint8_t lives[NUM_BUNKER_AREAS];
};

static struct bunker_t bunkers[NUM_BUNKERS];

// prints a damaged bunker block
// param[in] bunker the bunker number
// param[in] bunker_area the bunker area
// param[in] y the y coord of the block
// param[in] x the x coord of the block
void bunkers_print_damaged_block(uint8_t bunker, enum bunker_area_t bunker_area,
                                 uint16_t y, uint16_t x) {
  uint32_t bunker_block_sprite[BUNKER_BLOCK_SPRITE_SIZE];
  // set the sprite based on the damage of the bunker
  switch (bunkers[bunker].lives[bunker_area]) {
  case MININALLY_DAMAGED:
    bunker_block_sprite[ELEM0] = sprite_bunkerDamage2_6x6[ELEM0];
    bunker_block_sprite[ELEM1] = sprite_bunkerDamage2_6x6[ELEM1];
    bunker_block_sprite[ELEM2] = sprite_bunkerDamage2_6x6[ELEM2];
    bunker_block_sprite[ELEM3] = sprite_bunkerDamage2_6x6[ELEM3];
    bunker_block_sprite[ELEM4] = sprite_bunkerDamage2_6x6[ELEM4];
    bunker_block_sprite[ELEM5] = sprite_bunkerDamage2_6x6[ELEM5];
    break;
  case DAMAGED:
    bunker_block_sprite[ELEM0] = sprite_bunkerDamage1_6x6[ELEM0];
    bunker_block_sprite[ELEM1] = sprite_bunkerDamage1_6x6[ELEM1];
    bunker_block_sprite[ELEM2] = sprite_bunkerDamage1_6x6[ELEM2];
    bunker_block_sprite[ELEM3] = sprite_bunkerDamage1_6x6[ELEM3];
    bunker_block_sprite[ELEM4] = sprite_bunkerDamage1_6x6[ELEM4];
    bunker_block_sprite[ELEM5] = sprite_bunkerDamage1_6x6[ELEM5];
    break;
  case HEAVILY_DAMAGED:
    bunker_block_sprite[ELEM0] = sprite_bunkerDamage0_6x6[ELEM0];
    bunker_block_sprite[ELEM1] = sprite_bunkerDamage0_6x6[ELEM1];
    bunker_block_sprite[ELEM2] = sprite_bunkerDamage0_6x6[ELEM2];
    bunker_block_sprite[ELEM3] = sprite_bunkerDamage0_6x6[ELEM3];
    bunker_block_sprite[ELEM4] = sprite_bunkerDamage0_6x6[ELEM4];
    bunker_block_sprite[ELEM5] = sprite_bunkerDamage0_6x6[ELEM5];
    break;
  case GONE:
    bunker_block_sprite[ELEM0] = 0U;
    bunker_block_sprite[ELEM1] = 0U;
    bunker_block_sprite[ELEM2] = 0U;
    bunker_block_sprite[ELEM3] = 0U;
    bunker_block_sprite[ELEM4] = 0U;
    bunker_block_sprite[ELEM5] = 0U;
    break;
  default:
    break;
  };

  // cut off part of the sprite to make it the right shape
  if (bunker_area == TOP_1) {
    bunker_block_sprite[ELEM0] &= sprite_bunker_upper_left_gone_6x6[ELEM0];
    bunker_block_sprite[ELEM1] &= sprite_bunker_upper_left_gone_6x6[ELEM1];
    bunker_block_sprite[ELEM2] &= sprite_bunker_upper_left_gone_6x6[ELEM2];
    bunker_block_sprite[ELEM3] &= sprite_bunker_upper_left_gone_6x6[ELEM3];
    bunker_block_sprite[ELEM4] &= sprite_bunker_upper_left_gone_6x6[ELEM4];
    bunker_block_sprite[ELEM5] &= sprite_bunker_upper_left_gone_6x6[ELEM5];
  }

  // cut off part of the sprite to make it the right shape
  else if (bunker_area == TOP_4) {
    bunker_block_sprite[ELEM0] &= sprite_bunker_upper_right_gone_6x6[ELEM0];
    bunker_block_sprite[ELEM1] &= sprite_bunker_upper_right_gone_6x6[ELEM1];
    bunker_block_sprite[ELEM2] &= sprite_bunker_upper_right_gone_6x6[ELEM2];
    bunker_block_sprite[ELEM3] &= sprite_bunker_upper_right_gone_6x6[ELEM3];
    bunker_block_sprite[ELEM4] &= sprite_bunker_upper_right_gone_6x6[ELEM4];
    bunker_block_sprite[ELEM5] &= sprite_bunker_upper_right_gone_6x6[ELEM5];
  }
  // cut off part of the sprite to make it the right shape
  else if (bunker_area == MID_2) {
    bunker_block_sprite[ELEM0] &= sprite_bunker_lower_right_gone_6x6[ELEM0];
    bunker_block_sprite[ELEM1] &= sprite_bunker_lower_right_gone_6x6[ELEM1];
    bunker_block_sprite[ELEM2] &= sprite_bunker_lower_right_gone_6x6[ELEM2];
    bunker_block_sprite[ELEM3] &= sprite_bunker_lower_right_gone_6x6[ELEM3];
    bunker_block_sprite[ELEM4] &= sprite_bunker_lower_right_gone_6x6[ELEM4];
    bunker_block_sprite[ELEM5] &= sprite_bunker_lower_right_gone_6x6[ELEM5];

    // print twice
    hdmi_draw_sprite(bunker_block_sprite, BUNKER_BLOCK_SIZE, BUNKER_BLOCK_SIZE,
                     SCALE, fore2, back, y, x);
    x += BUNKER_BLOCK_SIZE * SCALE;

    // cut off part of the sprite to make it the right shape
    bunker_block_sprite[ELEM0] &= sprite_bunker_lower_left_gone_6x6[ELEM0];
    bunker_block_sprite[ELEM1] &= sprite_bunker_lower_left_gone_6x6[ELEM1];
    bunker_block_sprite[ELEM2] &= sprite_bunker_lower_left_gone_6x6[ELEM2];
    bunker_block_sprite[ELEM3] &= sprite_bunker_lower_left_gone_6x6[ELEM3];
    bunker_block_sprite[ELEM4] &= sprite_bunker_lower_left_gone_6x6[ELEM4];
    bunker_block_sprite[ELEM5] &= sprite_bunker_lower_left_gone_6x6[ELEM5];
  }

  // print damaged bunker
  hdmi_draw_sprite(bunker_block_sprite, BUNKER_BLOCK_SIZE, BUNKER_BLOCK_SIZE,
                   SCALE, fore2, back, y, x);
}

// called when a section of a bunker is hit
// @param[in] bunker_num number of the bunker that was hit
// @param[in] bunker_area the area of the bunker that was hit
void bunkers_hit(uint8_t bunker_num, enum bunker_area_t bunker_area) {
  if (bunkers[bunker_num].lives[bunker_area] == 0) {
    return;
  }

  bunkers[bunker_num].lives[bunker_area]--;

  uint16_t x_pos = BUNKER_1_X_POS + bunker_num * BUNKER_DISTANCE;
  uint16_t x_offset = 0;
  uint16_t y_pos = BUNKER_Y_POS;
  uint16_t y_offset = 0;

  // set y offset
  if (bunker_area == MID_1 || bunker_area == MID_2 || bunker_area == MID_3) {
    y_offset = BUNKER_BLOCK_SIZE * SCALE;
  } else if (bunker_area == BOT_1 || bunker_area == BOT_2) {
    y_offset = BUNKER_BLOCK_SIZE * SCALE + BUNKER_BLOCK_SIZE * SCALE;
  }

  // set x offset
  if (bunker_area <= TOP_4) {
    x_offset = bunker_area * BUNKER_BLOCK_SIZE * SCALE;
  } else if (bunker_area == MID_2) {
    x_offset = BUNKER_BLOCK_SIZE * SCALE;
  } else if (bunker_area == MID_3) {
    x_offset = 3 * BUNKER_BLOCK_SIZE * SCALE;
  } else if (bunker_area == BOT_2) {
    x_offset = 3 * BUNKER_BLOCK_SIZE * SCALE;
  }

  bunkers_print_damaged_block(bunker_num, bunker_area, y_pos + y_offset,
                              x_pos + x_offset);
}

// draw the bunkers and refill their lives
void bunkers_init() {

  hdmi_draw_sprite(sprite_bunker_24x18, BUNKER_LEN, BUNKER_HEIGHT, SCALE, fore2,
                   back, BUNKER_Y_POS, BUNKER_1_X_POS);
  hdmi_draw_sprite(sprite_bunker_24x18, BUNKER_LEN, BUNKER_HEIGHT, SCALE, fore2,
                   back, BUNKER_Y_POS, BUNKER_2_X_POS);
  hdmi_draw_sprite(sprite_bunker_24x18, BUNKER_LEN, BUNKER_HEIGHT, SCALE, fore2,
                   back, BUNKER_Y_POS, BUNKER_3_X_POS);
  hdmi_draw_sprite(sprite_bunker_24x18, BUNKER_LEN, BUNKER_HEIGHT, SCALE, fore2,
                   back, BUNKER_Y_POS, BUNKER_4_X_POS);

  // for each bunker
  for (uint8_t bunker = 0; bunker < NUM_BUNKERS; ++bunker) {
    // for each block
    for (uint8_t bunker_area = 0; bunker_area < NUM_BUNKER_AREAS;
         ++bunker_area) {
      bunkers[bunker].lives[bunker_area] = BUNKER_LIVES;
    }
  }
}

// checks if a bullet has hit a bunker
// @param[in] bullet_x the x position of a bullet
// @param[in] bullet_y the y position of a bullet
// @return true if the bullet hit a bunker, false if not
bool bunkers_collision(uint16_t bullet_x, uint16_t bullet_y) {
  // for each bunker
  for (uint8_t bunker = 0; bunker < NUM_BUNKERS; ++bunker) {
    // for each column
    for (uint8_t area_x = 0; area_x < BUNKER_LENGTH_BLOCKS; ++area_x) {
      // for each row
      for (uint8_t area_y = 0; area_y < BUNKER_HEIGHT_BLOCKS; ++area_y) {

        enum bunker_area_t bunker_area = NONE;
        // check the row of the block
        if (area_y == BUNKER_ROW0) {
          // check the column of the block
          if (area_x == BUNKER_COL0) {
            bunker_area = TOP_1;
          } else if (area_x == BUNKER_COL1) {
            bunker_area = TOP_2;
          } else if (area_x == BUNKER_COL2) {
            bunker_area = TOP_3;
          } else if (area_x == BUNKER_COL3) {
            bunker_area = TOP_4;
          }
        } else if (area_y == BUNKER_ROW1) {
          // check the column of the block
          if (area_x == BUNKER_COL0) {
            bunker_area = MID_1;
          } else if (area_x == BUNKER_COL1 || area_x == BUNKER_COL2) {
            bunker_area = MID_2;
          } else if (area_x == BUNKER_COL3) {
            bunker_area = MID_3;
          }
        } else if (area_y == BUNKER_ROW2) {
          // check the column of the block
          if (area_x == BUNKER_COL0) {
            bunker_area = BOT_1;
          } else if (area_x == BUNKER_COL3) {
            bunker_area = BOT_2;
          }
        }

        // if the bullet landed in a bunker area and the bunker has lives
        if (bunker_area != NONE && bunkers[bunker].lives[bunker_area] > 0) {
          uint16_t x_min = bunker * BUNKER_DISTANCE +
                           area_x * BUNKER_BLOCK_SIZE * SCALE - 2 * SCALE +
                           BUNKER_1_X_POS;
          uint16_t x_max = bunker * BUNKER_DISTANCE +
                           area_x * BUNKER_BLOCK_SIZE * SCALE + 6 * SCALE +
                           BUNKER_1_X_POS;
          uint16_t y_min = BUNKER_Y_POS + area_y * BUNKER_BLOCK_SIZE * SCALE;
          uint16_t y_max = y_min + BUNKER_BLOCK_SIZE * SCALE;

          // if the bullet hit the bunker
          if (bullet_x >= x_min && bullet_x <= x_max &&
              (BULLET_OFFSET + bullet_y) >= y_min && bullet_y <= y_max) {
            bunkers_hit(bunker, bunker_area);
            return true;
          }
        }
      }
    }
  }

  return false;
}

// tick function for the bunkers
void bunkers_tick() {
  // for each bunker
  for (uint8_t bunker = 0; bunker < NUM_BUNKERS; ++bunker) {
    uint16_t bullet_x;
    uint16_t bullet_y;

    bullet_get_player_bullet_coordinates(&bullet_x, &bullet_y);
    // if the player bullet hit the bunker
    if (bunkers_collision(bullet_x, bullet_y + BULLET_OFFSET)) {
      bullet_set_player_bullet_collide();
    }

    // for each alien bullet
    for (uint8_t bullet = 0; bullet < ALIEN_BULLETS; ++bullet) {
      bullet_get_alien_bullet_coordinates(&bullet_x, &bullet_y, bullet);
      // if the alien bullet hit a bunker
      if (bunkers_collision(bullet_x, bullet_y)) {
        bullet_set_alien_bullet_collide(bullet);
      }
    }
  }
}
