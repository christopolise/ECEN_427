#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Setter that can be called externally to cause game to end
void globals_set_game_over();

// Getter that will be called externally to poll whether the game has ended or
// not RETURN true if the game is over or false if not
bool globals_get_game_over();

// Getter that provides access to the global score
// RETURN uint16_t with the score of game
uint16_t globals_getScore();

// Setter that allows external modification of game score
// @param - score is a uint16_t that will modify the score of game
void globals_setScore(uint16_t score);

// Getter that will retrieve the lives remaining
// RETURN: uint8_t which will be the number of lives
uint8_t globals_getLives();

// Setter that uses the parameter as the new value of lives
// @param - lives is a uint8_t for the new value of global_lives
void globals_setLives(uint8_t lives);

// Returns whether an explosion is playing or not
bool globals_isExplosionPlayed();

// Returns whether saucer noise is enabled
bool globals_isSaucerPlayed();

// Returns whether bullet sound is playing
bool globals_isBulletPlayed();

// Returns whether the walks are being played
bool globals_isWalkPlayed();

// Returns whether audio is looping or not
bool globals_isLooping();

// Setter for indicating explosion is playing
void globals_setExplosionPlaying(bool enable);

// Setter for indicating whether saucer is really on screen
void globals_setSaucerPlaying(bool enable);

// Setter for indicating whether bullet sound is playing
void globals_setBulletPlaying(bool enable);

// Setter for indicating whether the walking sounds are playing
void globals_setWalkPlaying(bool enable);

// Setter for enabling or disabling looping
void globals_setLooping(bool enable);

// Runs all necessary flag assignments and default values for the handling of a
// game
void globals_init();

// Steps through the states to make sure scores, lives, and other data is
// updated on a tick
void globals_tick();

#endif