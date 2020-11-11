#include "globals.h"
#include "hdmi.h"
#include "sprites.h"
#include <stdlib.h>

#define INCREMENT 1
#define DECREMENT -1
#define TANK_WIDTH 17
#define TANK_HEIGHT 8
#define SCALE_FACTOR 2
#define LIVES_MAX 5
#define SCORE_BUF_LEN 10
#define SCORE_SIZE 6
#define SCALE_LIFE 1
#define SCORE_POS_Y 10
#define SCORE_POS_X 75
#define LIVES_POS_Y 10
#define LIVES_START_POS 135
#define LIVES_WIDTH 20
#define COL_ALL_OFF 0x00
#define COL_ALL_ON 0xFF
#define TANK_R 0x78
#define TANK_G 0xB9
#define TANK_B 0xBF
#define DEFAULT_LIVES 3
#define DEFAULT_SCORE 0

enum globalsControlsMachine {
  init_st,           // Machine is disabled
  wait_for_event_st, // Wait for game to raise event
  update_lives_st,   // Update lives count
  update_score_st    // Update score count
} currentGlobalsHandler;

static bool enableGlobals;
static bool livesFlag;
static bool scoreFlag;
static bool gameOver;

static bool saucerIsPlaying = false;
static bool explosionIsPlaying = false;
static bool bulletIsPlaying = false;
static bool walkIsPlaying = false;
static bool soundIsLooping = false;

static uint8_t globals_lives = DEFAULT_LIVES;
static uint8_t globals_old_lives;
static uint16_t globals_score = DEFAULT_SCORE;
static uint16_t globals_old_score;
char globalBack[HDMI_COLOR_FACTOR] = {COL_ALL_OFF, COL_ALL_OFF, COL_ALL_OFF};
char globalFore[HDMI_COLOR_FACTOR] = {COL_ALL_OFF, COL_ALL_ON, COL_ALL_OFF};
char globalTank[HDMI_COLOR_FACTOR] = {TANK_R, TANK_G, TANK_B};

bool globals_isExplosionPlayed() { return explosionIsPlaying; }

bool globals_isSaucerPlayed() { return saucerIsPlaying; }

bool globals_isBulletPlayed() { return bulletIsPlaying; }

bool globals_isWalkPlayed() { return walkIsPlaying; }

bool globals_isLooping() { return soundIsLooping; }

void globals_setExplosionPlaying(bool enable) { explosionIsPlaying = enable; }

void globals_setSaucerPlaying(bool enable) { saucerIsPlaying = enable; }

void globals_setBulletPlaying(bool enable) { bulletIsPlaying = enable; }

void globals_setWalkPlaying(bool enable) { walkIsPlaying = enable; }

void globals_setLooping(bool enable) { soundIsLooping = enable; }

// Setter that can be called externally to cause game to end
void globals_set_game_over() { gameOver = true; }

// Getter that will be called externally to poll whether the game has ended or
// not RETURN true if the game is over or false if not
bool globals_get_game_over() { return gameOver; }

// Getter that provides access to the global score
// RETURN uint16_t with the score of game
uint16_t globals_getScore() { return globals_score; }

// Setter that allows external modification of game score
// @param - score is a uint16_t that will modify the score of game
void globals_setScore(uint16_t score) {
  // Keeps value of old score for erasing purposes
  scoreFlag = true;
  globals_old_score = globals_score;
  globals_score = score;
}

// Getter that will retrieve the lives remaining
// RETURN: uint8_t which will be the number of lives
uint8_t globals_getLives() { return globals_lives; }

// Setter that uses the parameter as the new value of lives
// @param - lives is a uint8_t for the new value of global_lives
void globals_setLives(uint8_t lives) {
  // Keeps value of old lives for erasing purposes
  livesFlag = true;
  globals_old_lives = globals_lives;
  globals_lives = lives;

  // Keep at max limit if already at max limit
  if (globals_lives > LIVES_MAX)
    globals_lives = LIVES_MAX;

  // Trigger a game over event if lives hits zero
  if (!globals_lives) {
    gameOver = true;
    return;
  }
}

// Graphical routine to erase the old score and draw over it with the new one
void update_score() {
  scoreFlag = false;
  char score_old[SCORE_BUF_LEN];
  char score[SCORE_BUF_LEN];

  // Load string with the old and new score values
  snprintf(score_old, SCORE_SIZE, "%05d", globals_old_score);
  snprintf(score, SCORE_SIZE, "%05d", globals_score);

  // Erase old score and populate the new one
  hdmi_print_string(score_old, strlen(score), SCALE_FACTOR, globalBack,
                    globalBack, SCORE_POS_Y, SCORE_POS_X);
  hdmi_print_string(score, strlen(score), SCALE_FACTOR, globalFore, globalBack,
                    SCORE_POS_Y, SCORE_POS_X);
}

// Graphical routine to add or take away a tank based off of updated lives count
void update_lives() {
  livesFlag = false;

  // Mathematical way to determine whether inc or dec
  int8_t inc_or_dec = globals_lives - globals_old_lives;

  // Score incremented, adds tank
  if (inc_or_dec == INCREMENT) {
    hdmi_draw_sprite(
        sprite_tank_17x8, TANK_WIDTH, TANK_HEIGHT, SCALE_LIFE, globalTank,
        globalBack, LIVES_POS_Y,
        HDMI_DISPLAY_WIDTH -
            (LIVES_START_POS + (LIVES_WIDTH - globals_lives * LIVES_WIDTH)));
  } else if (inc_or_dec == DECREMENT) {
    // Score decremented, takes away tank
    hdmi_draw_sprite(sprite_tank_17x8, TANK_WIDTH, TANK_HEIGHT, SCALE_LIFE,
                     globalBack, globalBack, LIVES_POS_Y,
                     HDMI_DISPLAY_WIDTH -
                         (LIVES_START_POS +
                          (LIVES_WIDTH - globals_old_lives * LIVES_WIDTH)));
  }
}

// Runs all necessary flag assignments and default values for the handling of a
// game
void globals_init() {
  // Resets flags to default values and starts FSM
  enableGlobals = true;
  livesFlag = false;
  scoreFlag = false;
  gameOver = false;
  currentGlobalsHandler = init_st;
}

// Steps through the states to make sure scores, lives, and other data is
// updated on a tick
void globals_tick() {
  // Mealy Actions
  switch (currentGlobalsHandler) {
  case init_st: // Waiting to start FSM
    // start entrance into FSM if enabled
    if (enableGlobals) {
      currentGlobalsHandler = wait_for_event_st;
    }
    break;

  case wait_for_event_st: // Waiting for life to throw something my way
    // If the lives have been updated, then run routine to update screen
    if (livesFlag) {
      currentGlobalsHandler = update_lives_st;
    } else if (scoreFlag) {
      // If the score has been updated, run the appropriate routine
      currentGlobalsHandler = update_score_st;
    }
    break;

  case update_lives_st: // Get a life...or lose one, then goes back to waiting
    currentGlobalsHandler = wait_for_event_st;
    break;

  case update_score_st: // Updates the score then goes back to waiting
    currentGlobalsHandler = wait_for_event_st;
    break;
  }

  // Moore Actions
  switch (currentGlobalsHandler) {
  case init_st: // No fanciness here
    break;

  case wait_for_event_st: // Wait patiently
    break;

  case update_lives_st: // Draws updates to screen
    update_lives();
    break;

  case update_score_st: // Draws updated score to the screen
    update_score();
    break;
  }
}