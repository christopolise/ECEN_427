#ifndef __BULLET_H__
#define __BULLET_H__

#include <stdbool.h>
#include <stdint.h>

#define ALIEN_BULLETS 4

// Gets the active status of a specified alien bullet
// @param - num_of_alien is a uint8_t that denotes which alien bullet we are querying
// RETURN: true if the alien bullet is active false otherwise
bool bullet_is_alien_bullet_active(uint8_t num_of_alien);

// Fires a bullet from the current tank location as indicated in player
// Ensures that bullet is inactive on screen before shooting again
// @param - x_pos is uint16_t that describes x-coordinate of bullet origin
// @param - y_pos is uint16_t that describes y-coordinate of bullet origin
void bullet_player_fire(uint16_t x_pos, uint16_t y_pos);

// Determines which alien bullet out of the 4 choices available will fire the bullet and where
// @param - alien_num is uint8_t that describes which of the 4 active alien bullets is being
// referenced
// @param - x_pos is uint16_t that describes x-coordinate of new bullet origin
// @param - y_pos is uint16_t that describes y-coordinate of new bullet origin
void bullet_alien_fire(uint8_t alien_num, uint16_t x_pos, uint16_t y_pos);

// Lets the bullet drawing system know that the player bullet has collided
// with something
// @param - num_of_alien is uint8_t that describes which of the 4 active alien bullets is being
// referenced
void bullet_set_alien_bullet_collide(uint8_t num_of_alien);

// Lets the bullet drawing system know that the player bullet has collided
// with something
void bullet_set_player_bullet_collide();

bool bullet_get_player_bullet_is_alive();

// Lets the caller set the coordinates of the player bullet
// @param - x_pos uint16_t that is part of the new player coordinate
// @param - y_pos uint16_t that is part of the new player coordinate
void bullet_set_player_bullet_coordinates(uint16_t x_pos, uint16_t y_pos);

// Passes the values of the player's coordinates to the addresses entered
// into the parameters
// @param - x_pos is uint16_t * that will receive x-coordinate of bullet
// @param - y_pos is uint16_t * that will receive y-coordinate of bullet
void bullet_get_player_bullet_coordinates(uint16_t *x_pos, uint16_t *y_pos);

// Allows caller to set the coordinates of the starting point of an alien shot. The purpose is
// to allow caller to do their own randomization of shots and use this function as the interface
// to actually locate and shoot bullet from coordinates
// @param - x_pos is uint16_t that describes x-coordinate of new bullet origin
// @param - y_pos is uint16_t that describes y-coordinate of new bullet origin
// @param - num_of_alien is uint8_t that describes which of the 4 active alien bullets is being
// referenced
void bullet_set_alien_bullet_coordinates(uint16_t x_pos, uint16_t y_pos, uint8_t num_of_alien);

// Retreives the coordinates of an alien bullet given an index in the bullet array
// @param - x_pos is uint16_t * that will receive x-coordinate of bullet
// @param - y_pos is uint16_t * that will receive y-coordinate of bullet
// @param - num_of_alien is uint8_t that describes which of the 4 active alien bullets is being
// referenced
void bullet_get_alien_bullet_coordinates(uint16_t *x_pos, uint16_t *y_pos, uint8_t num_of_alien);

// Initializes the state machine that keeps track of all bullet events
void bullet_init();

// Executes state machine that determines bullet behavior
void bullet_tick();

#endif