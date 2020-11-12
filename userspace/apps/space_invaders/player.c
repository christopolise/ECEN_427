#include "player.h"
#include "bullet.h"
#include "globals.h"
#include "sprites.h"
#include "sounds.h"
#include <buttons/buttons.h>

#define START_POS_X HDMI_DISPLAY_WIDTH / 2 - 8
#define POS_Y HDMI_DISPLAY_HEIGHT - 25
#define SCALE 2
#define RIGHT_EDGE HDMI_DISPLAY_WIDTH - 1 - (17 * SCALE)
#define TANK_WIDTH 17
#define TANK_NORM_WIDTH 15
#define TANK_HEIGHT 8
#define EXPLOSION_CNT_MAX 70
#define EXPLOSION_SWTCH_INT 10
#define CNT_RESET 0
#define ALIEN_BULLETS 4
#define GROUND_OFFSET 10
#define COL_ALL_OFF 0x00
#define TANK_COL_R 0x78
#define TANK_COL_G 0xB9
#define TANK_COL_B 0xBF
#define BULLET_LENGTH 5
#define ALIEN_BULLET_WIDTH 3

typedef struct player_tank_t {
  uint16_t pos_x;
  uint16_t pos_y;
  bool is_alive;
  uint8_t lives;
} p_tank;

// Global variables for the state machine
static struct player_tank_t player_tank;
static uint8_t explosion_counter = 0;
static bool enablePlayer = false;
static bool explosion1 = true;
static char tank[HDMI_COLOR_FACTOR] = {TANK_COL_R, TANK_COL_G, TANK_COL_B};
static char back[HDMI_COLOR_FACTOR] = {COL_ALL_OFF, COL_ALL_OFF, COL_ALL_OFF};

enum playerControlsMachine {
  init_st,           // Machine is disabled
  wait_for_input_st, // Waiting for user input
  move_st,           // Moves the tank on the screen
  fire_st,           // Fires a round at the aliens
  hit_received_st    // Registers a hit from the aliens
} currentPlayerHandler;

// This function sets up all the preliminary variables to make sure the state
// machine is in the right condition to start
void player_init() {
  currentPlayerHandler = init_st; // Puts state machine in initial state
  enablePlayer = true;
  player_tank.pos_x = START_POS_X;
  player_tank.pos_y = POS_Y;
  player_tank.is_alive = true;
  player_tank.lives = globals_getLives();
  hdmi_draw_sprite(sprite_tank_17x8, TANK_WIDTH, TANK_HEIGHT, SCALE, tank, back,
                   player_tank.pos_y, player_tank.pos_x);
}

// Moves the tank on screen by taking its current position and either
// incrementing or decrementing the current x position based on buttons input
// @param - buttons is a uint32_t that when masked with the right #define
// statement will return t/f whether a specific button is pressed
void move_tank(uint32_t buttons) {
  // If button 3 is one of the buttons pressed
  if (buttons & BUTTONS_2_MASK) {
    // If the tank is at the extreme left of the screen
    if (!player_tank.pos_x) {
      // Keeps the tank stationary and limited
      player_tank.pos_x = 0;
    }
    // Somewhere in the middle
    else {
      // Decreases the x position and moves tank left
      player_tank.pos_x--;
    }
  }
  // If button 2 is one of the buttons pressed
  else if (buttons & BUTTONS_1_MASK) {
    // If the tanks edge is touching the right end of the screen
    if (player_tank.pos_x == RIGHT_EDGE) {
      // We keep the tank stationary and limited
      player_tank.pos_x = RIGHT_EDGE;
    }
    // Is somewhere in the middle
    else {
      // We increase the x position and make the tank move right
      player_tank.pos_x++;
    }
  }
  // Draws tank in calculated position
  hdmi_draw_sprite(sprite_tank_17x8, TANK_WIDTH, TANK_HEIGHT, SCALE, tank, back,
                   player_tank.pos_y, player_tank.pos_x);
}

// Fires a bullet from the tanks current position. Offsets the starting point a
// little to maintain the image of firing from the turret
void fire_bullet() {
  bullet_player_fire(player_tank.pos_x + TANK_HEIGHT * SCALE,
                     player_tank.pos_y - GROUND_OFFSET);
}

// Polls all the alien bullets to see if any are currently within the range and
// position of the tank in this tick RETURN: True if a bullet struck the tank,
// false otherwise
bool check_bullet() {
  // For all the possible active alien bullets
  for (uint8_t i = 0; i < ALIEN_BULLETS; i++) {
    uint16_t x_pos;
    uint16_t y_pos;
    bullet_get_alien_bullet_coordinates(&x_pos, &y_pos, i);
    // If the bottom of the selected alien bullet is within the correct height
    // and x span of the tank
    if ((y_pos + GROUND_OFFSET >= player_tank.pos_y) &&
        (x_pos + ALIEN_BULLET_WIDTH >= player_tank.pos_x &&
         x_pos - ALIEN_BULLET_WIDTH <=
             (player_tank.pos_x + TANK_WIDTH * SCALE))) {
      // Report a hit
      bullet_set_alien_bullet_collide(i);
      player_tank.lives--;
      globals_setLives(player_tank.lives);
      return true;
    }
  }

  // No hit reported
  return false;
}

// Handles the animation of the tank explosion by alternating sprites on a timer
void explosion() {
  // If the tank was recently hit but not yet switched to dead
  if (player_tank.is_alive) {
    // Erase tank and declare dead
    hdmi_draw_sprite(sprite_tank_17x8, TANK_WIDTH, TANK_HEIGHT, SCALE, back,
                     back, player_tank.pos_y, player_tank.pos_x);
    player_tank.is_alive = false;
  }

  // If the first sprite is in use
  if (explosion1) {
    // Erase second sprite and print 1
    hdmi_draw_sprite(sprite_tank_explosion2_15x8, TANK_NORM_WIDTH, TANK_HEIGHT,
                     SCALE, back, back, player_tank.pos_y, player_tank.pos_x);
    hdmi_draw_sprite(sprite_tank_explosion1_15x8, TANK_NORM_WIDTH, TANK_HEIGHT,
                     SCALE, tank, back, player_tank.pos_y, player_tank.pos_x);
  }
  // If second sprite is in use
  else {
    // Erase first sprite and draw second
    hdmi_draw_sprite(sprite_tank_explosion1_15x8, TANK_NORM_WIDTH, TANK_HEIGHT,
                     SCALE, back, back, player_tank.pos_y, player_tank.pos_x);
    hdmi_draw_sprite(sprite_tank_explosion2_15x8, TANK_NORM_WIDTH, TANK_HEIGHT,
                     SCALE, tank, back, player_tank.pos_y, player_tank.pos_x);
  }

  // Toggle flag to enable change of sprites
  explosion1 = !explosion1;
}

// Carries out the needed Mealy and Moore actions based off the given flags and
// states
void player_tick() {

  uint32_t buttons = buttons_read();

  // Mealy Actions
  switch (currentPlayerHandler) {
  case init_st: // Machine is disabled
    // If the machine has been initialized continue to wait for input
    if (enablePlayer) {
      currentPlayerHandler = wait_for_input_st;
    }
    // Stay in current state
    else {
      currentPlayerHandler = init_st;
    }
    break;

  case wait_for_input_st: // Waiting for user input
    // If a bullet struck the player, go to state
    if (check_bullet()) {
      currentPlayerHandler = hit_received_st;
    }

    // If either button 2 or 3 are pushed (the mvmnt btns), go to move state
    if ((buttons & BUTTONS_2_MASK) || (buttons & BUTTONS_1_MASK)) {
      currentPlayerHandler = move_st;
    }
    // If the fire button is pushed go to fire state
    else if (buttons & BUTTONS_0_MASK) {
      currentPlayerHandler = fire_st;
    }
    break;

  case move_st: // Moves tank on screen

    // If a bullet has struck the tank, go to hit state
    if (check_bullet()) {
      currentPlayerHandler = hit_received_st;
    }

    // If fire button is pressed, go to fire state
    if (buttons & BUTTONS_0_MASK) {
      currentPlayerHandler = fire_st;
      if(!globals_isExplosionPlayed() && !globals_isBulletPlayed() && !bullet_get_player_bullet_is_alive())
    {
      globals_setBulletPlaying(true);
      sounds_play(SOUNDS_LASER_INDX);
      globals_setBulletPlaying(false);
    }
    }
    // Go back to waiting for input
    else {
      currentPlayerHandler = wait_for_input_st;
    }
    break;

  case fire_st: // Fires bullet upwards
    // If a bullet is striking the tank, go to hit state
    if (check_bullet()) {
      currentPlayerHandler = hit_received_st;
    }
    // Go back to waiting for input after shots fired
    currentPlayerHandler = wait_for_input_st;
    break;

  case hit_received_st: // Have been struck by a bullet
    // If the duration of explosion animation has maxed out, reset counters,
    // tank position, and flags, go to init

    printf("SOUND PLAYED\n");
    if(explosion_counter ==1)
    {
      printf("SOUND PLAYED\n");
      globals_setExplosionPlaying(true);
      sounds_play(SOUNDS_PLAYER_DIE_INDX);
      globals_setExplosionPlaying(false);
    }

    if (explosion_counter == EXPLOSION_CNT_MAX) {
      explosion_counter = CNT_RESET;
      hdmi_draw_sprite(sprite_tank_gone_15x8, TANK_NORM_WIDTH, TANK_HEIGHT,
                       SCALE, back, back, player_tank.pos_y, player_tank.pos_x);
      player_tank.is_alive = true;
      explosion1 = true;
      currentPlayerHandler = init_st;
    }
    break;

  default: // No action here
    break;
  }

  // Moore Actions
  switch (currentPlayerHandler) {
  case init_st: // Initialization state
    player_init();
    break;

  case wait_for_input_st: // Functions to run while waiting
    break;

  case move_st: // Move the tank based on input
    move_tank(buttons);
    break;

  case fire_st: // Fire bullet if possible
    fire_bullet();
    break;

  case hit_received_st: // Hit by bullet
    // If the counter is divisible by 10, switch animation and call draw funct
    if (!(explosion_counter % EXPLOSION_SWTCH_INT)) {
      explosion();
    }
    // Keep incrementing that counter
    explosion_counter++;
    break;

  default: // Nothing to see here
    break;
  }
}
