#include "bullet.h"
#include "hdmi.h"
#include "sprites.h"
#include <stdio.h>

#define SCALE_FACTOR 2
#define BULLET_LENGTH 5
#define ALIEN_BULLET_WIDTH 3
#define PLAYER_BULLET_WIDTH 1
#define GROUND_OFFSET 10
#define BANNER_BOUNDARY 30
#define EVEN_FACTOR 2
#define ALIEN_SPEED 2
#define GRASS_LIM 6

enum bulletControlsMachine {
    init_st, // Machine is disabled
    fly_st   // Waiting for user input
} currentBulletHandler;

typedef struct player_bullet_t {
    uint16_t pos_x;
    uint16_t pos_y;
    bool is_alive;
} p_bullet;

typedef struct alien_bullet_t {
    uint16_t pos_x;
    uint16_t pos_y;
    bool is_up;
    bool is_alive;
} a_bullet;

static struct player_bullet_t play_bullet;
static struct alien_bullet_t aliens_bullets[ALIEN_BULLETS];
static bool enableBulletHandler;
static char backgr[HDMI_COLOR_FACTOR] = {0x00, 0x00, 0x00};
static char fore[HDMI_COLOR_FACTOR] = {0xFF, 0x00, 0x00};

// Gets the active status of a specified alien bullet
// @param - num_of_alien is a uint8_t that denotes which alien bullet we are querying
// RETURN: true if the alien bullet is active false otherwise
bool bullet_is_alien_bullet_active(uint8_t num_of_alien) {
    return aliens_bullets[num_of_alien].is_alive;
}

// Lets the bullet drawing system know that the player bullet has collided
// with something
// @param - num_of_alien is uint8_t that describes which of the 4 active alien bullets is being
// referenced
void bullet_set_alien_bullet_collide(uint8_t num_of_alien) {
    hdmi_draw_sprite(sprite_alienbullet2_gone_3x5, ALIEN_BULLET_WIDTH, BULLET_LENGTH, SCALE_FACTOR,
                         backgr, backgr, aliens_bullets[num_of_alien].pos_y, aliens_bullets[num_of_alien].pos_x); 
    aliens_bullets[num_of_alien].is_alive = false;
    aliens_bullets[num_of_alien].pos_x = 0;
    aliens_bullets[num_of_alien].pos_y = 0;
}

// Lets the bullet drawing system know that a certain alien bullet has
// collided with an object
void bullet_set_player_bullet_collide() {
    hdmi_draw_sprite(sprite_tankbullet_1x5, PLAYER_BULLET_WIDTH, BULLET_LENGTH, SCALE_FACTOR,
                         backgr, backgr, play_bullet.pos_y, play_bullet.pos_x); 
    play_bullet.is_alive = false; 
    play_bullet.pos_x = 0;
    play_bullet.pos_y = 0;
}

// Passes the values of the player's coordinates to the addresses entered
// into the parameters
// @param - x_pos is uint16_t * that will receive x-coordinate of bullet
// @param - y_pos is uint16_t * that will receive y-coordinate of bullet
void bullet_get_player_bullet_coordinates(uint16_t *x_pos, uint16_t *y_pos) {
    *x_pos = play_bullet.pos_x;
    *y_pos = play_bullet.pos_y;
}

bool bullet_get_player_bullet_is_alive(){
    return play_bullet.is_alive;
}

// Retreives the coordinates of an alien bullet given an index in the bullet array
// @param - x_pos is uint16_t * that will receive x-coordinate of bullet
// @param - y_pos is uint16_t * that will receive y-coordinate of bullet
// @param - num_of_alien is uint8_t that describes which of the 4 active alien bullets is being
// referenced
void bullet_get_alien_bullet_coordinates(uint16_t *x_pos, uint16_t *y_pos, uint8_t num_of_alien) {
    *x_pos = aliens_bullets[num_of_alien].pos_x;
    *y_pos = aliens_bullets[num_of_alien].pos_y;
}

// Allows caller to set the coordinates of the starting point of an alien shot. The purpose is
// to allow caller to do their own randomization of shots and use this function as the interface
// to actually locate and shoot bullet from coordinates
// @param - x_pos is uint16_t that describes x-coordinate of new bullet origin
// @param - y_pos is uint16_t that describes y-coordinate of new bullet origin
// @param - num_of_alien is uint8_t that describes which of the 4 active alien bullets is being
// referenced
void bullet_set_alien_bullet_coordinates(uint16_t x_pos, uint16_t y_pos, uint8_t num_of_alien) {
    aliens_bullets[num_of_alien].pos_x = x_pos;
    aliens_bullets[num_of_alien].pos_y = y_pos;
}

// Lets the caller set the coordinates of the player bullet
// @param - x_pos uint16_t that is part of the new player coordinate
// @param - y_pos uint16_t that is part of the new player coordinate
void bullet_set_player_bullet_coordinates(uint16_t x_pos, uint16_t y_pos) {
    play_bullet.pos_x = x_pos;
    play_bullet.pos_y = y_pos;
}

// Fires a bullet from the current tank location as indicated in player
// Ensures that bullet is inactive on screen before shooting again
// @param - x_pos is uint16_t that describes x-coordinate of bullet origin
// @param - y_pos is uint16_t that describes y-coordinate of bullet origin
void bullet_player_fire(uint16_t x_pos, uint16_t y_pos) {
    if (!play_bullet.is_alive) {
        // There is no active player bullet on screen
        play_bullet.pos_y = y_pos;
        play_bullet.pos_x = x_pos;
        play_bullet.is_alive = true; // Enable active bullet to travel
    }
}

// Determines which alien bullet out of the 4 choices available will fire the bullet and where
// @param - alien_num is uint8_t that describes which of the 4 active alien bullets is being
// referenced
// @param - x_pos is uint16_t that describes x-coordinate of new bullet origin
// @param - y_pos is uint16_t that describes y-coordinate of new bullet origin
void bullet_alien_fire(uint8_t alien_num, uint16_t x_pos, uint16_t y_pos) {
    if (!aliens_bullets[alien_num].is_alive) {
        // This block will only run if the current alien bullet selected isn't already active
        aliens_bullets[alien_num].pos_x = x_pos;
        aliens_bullets[alien_num].pos_y = y_pos;
        aliens_bullets[alien_num].is_alive = true;
    }
}

// Initializes the state machine that keeps track of all bullet events
void bullet_init() {
    currentBulletHandler = init_st;
    enableBulletHandler = true;
    play_bullet.is_alive = false;
    play_bullet.pos_x = 0;
    play_bullet.pos_y = 0;

    for (uint8_t i = 0; i < ALIEN_BULLETS; i++) {
        // For all existing alien bullets, we will set their availablity and orientation to default
        // to false
        aliens_bullets[i].is_alive = false;
        aliens_bullets[i].is_up = false;
        aliens_bullets[i].pos_x = 0;
        aliens_bullets[i].pos_y = 0;
    }
}

// This function determines the rate and the trajectory a fired bullet from the tank has
void player_shot_fly() {
    // If the bullet has not reached the top of the score and lives toolbar
    if ((play_bullet.pos_y > BANNER_BOUNDARY) && play_bullet.is_alive) {
        // erase current bullet at position
        hdmi_draw_sprite(sprite_tankbullet_1x5, PLAYER_BULLET_WIDTH, BULLET_LENGTH, SCALE_FACTOR,
                         backgr, backgr, play_bullet.pos_y, play_bullet.pos_x);
        // Increment bullet's position
        play_bullet.pos_y -= BULLET_LENGTH;
        // Draw bullet
        hdmi_draw_sprite(sprite_tankbullet_1x5, PLAYER_BULLET_WIDTH, BULLET_LENGTH, SCALE_FACTOR,
                         fore, backgr, play_bullet.pos_y, play_bullet.pos_x);
    }
    // The bullet has reached the top of the score and lives toolbar
    else {
        // erase bullet at current position
        hdmi_draw_sprite(sprite_tankbullet_1x5, PLAYER_BULLET_WIDTH, BULLET_LENGTH, SCALE_FACTOR,
                         backgr, backgr, play_bullet.pos_y, play_bullet.pos_x);
        play_bullet.is_alive = false;
        play_bullet.pos_x = 0;
        play_bullet.pos_y = 0;
    }
}

// Helper function that moves along a bullet along a certain path and with a particular sprite or
// set of them, this is what should allow alien bullets to change shape
// @param - num_of_alien is uint8_t that describes which of the 4 active alien bullets is being
// referenced
// @param - sprite_name is an array of uint32_t which, when passed through a print sprite function,
// will print the bitmap onto the screen
void aliens_animation_helper(uint8_t num_of_alien, const uint32_t *sprite_name) {
    // If the bullet is still on the screen and is still registered as not having hit anything
    if ((aliens_bullets[num_of_alien].pos_y + GROUND_OFFSET < (HDMI_DISPLAY_HEIGHT - GRASS_LIM)) &&
        aliens_bullets[num_of_alien].is_alive) {
        // erase current bullet at position
        hdmi_draw_sprite(sprite_name, ALIEN_BULLET_WIDTH, BULLET_LENGTH, SCALE_FACTOR, backgr,
                         backgr, aliens_bullets[num_of_alien].pos_y,
                         aliens_bullets[num_of_alien].pos_x);
        // Increment bullet's position
        aliens_bullets[num_of_alien].pos_y += ALIEN_SPEED;
        // Draw bullet
        hdmi_draw_sprite(sprite_name, ALIEN_BULLET_WIDTH, BULLET_LENGTH, SCALE_FACTOR, fore, backgr,
                         aliens_bullets[num_of_alien].pos_y, aliens_bullets[num_of_alien].pos_x);
    }
    // Upon an object hit
    else {
        // erase bullet at current position
        hdmi_draw_sprite(sprite_name, ALIEN_BULLET_WIDTH, BULLET_LENGTH, SCALE_FACTOR, backgr,
                         backgr, aliens_bullets[num_of_alien].pos_y,
                         aliens_bullets[num_of_alien].pos_x);
        aliens_bullets[num_of_alien].is_alive = false;
        aliens_bullets[num_of_alien].pos_x = 0;
        aliens_bullets[num_of_alien].pos_y = 0;
    }
}

// Function that causes an alien bullet to fly and alternate the pattern as it flies and the type of
// bullet based on the eveness of the number
void aliens_shots_fly() {
    // For all the available random bullets that aliens can have on screen at once
    for (uint8_t i = 0; i < ALIEN_BULLETS; i++) {
        if(aliens_bullets[i].is_alive){
            // If the index number is even
            if (!(i % EVEN_FACTOR)) {
                // If the alien bullet type was just in the up position, switch to down
                if (aliens_bullets[i].is_up) {
                    aliens_animation_helper(i, sprite_alienbullet1_up_3x5);
                }
                // If the alien bullet type was just in the down position, switch to up
                else {
                    aliens_animation_helper(i, sprite_alienbullet1_down_3x5);
                }
            }
            // If the index number is odd
            else {
                // If the alien bullet type was just in the up position, switch to down
                if (aliens_bullets[i].is_up) {
                    aliens_animation_helper(i, sprite_alienbullet2_up_3x5);
                }
                // If the alien bullet type was just in the down position, switch to up
                else {
                    aliens_animation_helper(i, sprite_alienbullet2_down_3x5);
                }
            }
        }
    }
}

// Executes state machine that determines bullet behavior
void bullet_tick() {
    // Mealy Actions
    switch (currentBulletHandler) {
    case init_st: // The starting state
        // If the machine is enabled to be run, continue to bullet flying state
        if (enableBulletHandler) {
            currentBulletHandler = fly_st;
        }
        break;

    case fly_st: // While the bullet is in the air
        // If someone disables the machine, go back to the init state
        if (!enableBulletHandler) {
            currentBulletHandler = init_st;
        }
        break;

    default:
        break;
    }

    // Moore Actions
    switch (currentBulletHandler) {
    case init_st: // Any set up functions to be run
        break;

    case fly_st: // Calling all fly animations
        player_shot_fly();
        aliens_shots_fly();
        break;

    default:
        break;
    }
}