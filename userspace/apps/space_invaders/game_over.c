#include "game_over.h"
#include "button_debouncer.h"
#include "hdmi.h"
#include <buttons/buttons.h>
#include <fcntl.h>
#include <intc/intc.h>
#include <stdio.h>
#include <string.h>
#include <switches/switches.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <system.h>
#include <unistd.h>

#define LETTER1 0
#define LETTER2 1
#define LETTER3 2
#define FILE_SIZE 70
#define NAME_SIZE 3
#define NUM_SCORES 9
#define BLINK_LENGTH 25
#define FILE_NAME "/highscores"
#define FILE_DNE -1
#define MAX_SCORE_SIZE 10
#define INITIALS_FIELD_SIZE 4
#define GO_TITLE_SCALE 8
#define GO_Y_POS 50
#define GO_X_POS 100
#define INITIALS_SCALE 4
#define INITIALS_Y_POS 200
#define INITIALS_X_POS 280
#define INITIAL_OFFSET 110
#define INITIAL_SCALE_WID 40
#define INITIAL_FACTOR_1 170
#define INITIAL_FACTOR_2 350

typedef struct highscore_t {
    char name[NAME_SIZE];
    uint32_t score;
} null;

static struct highscore_t scoreboard[NUM_SCORES];

static char letter1 = 'A';
static char letter2 = 'A';
static char letter3 = 'A';
static uint8_t selected_letter = 0U;
static char black[HDMI_COLOR_FACTOR] = {0x7F, 0x00, 0xFF};
static char white[HDMI_COLOR_FACTOR] = {0xFF, 0xFF, 0xFF};

// Simple function that displays the name entered
// @param - message string that is the name to be input
void display_name(char *message) {
    static char old_msg[INITIALS_FIELD_SIZE] = {'\0', '\0', '\0', '\0'};
    hdmi_print_string((char *)old_msg, strlen((char *)old_msg), INITIALS_SCALE, black, black, INITIALS_Y_POS, INITIALS_X_POS);
    hdmi_print_string(message, strlen(message), INITIALS_SCALE, white, black, INITIALS_Y_POS, INITIALS_X_POS);
    old_msg[LETTER1] = message[LETTER1];
    old_msg[LETTER2] = message[LETTER2];
    old_msg[LETTER3] = message[LETTER3];
}

// Sets up the display by clearing screen and writing necessary components
bool set_display() {
    bool success = (hdmi_init(HDMI_DEVICE_FILE) == 0);
    if (!success) {
        // Exits program with error because the initialization of driver failed
        printf("hdmi_init failed\n");
    }

    hdmi_fill_screen(black);
    char *message = "GAME OVER";
    hdmi_print_string(message, strlen(message), GO_TITLE_SCALE, white, black, GO_Y_POS, GO_X_POS);

    char name[INITIALS_FIELD_SIZE] = {letter1, letter2, letter3, '\0'};
    display_name(name);

    printf("AAA");
    fflush(stdout);

    return success;
}

// Actualizes the highscore on file and keeps sorted
// @param score highscore_t that is placed in file
void update_highscores(struct highscore_t *score) {
    struct highscore_t new_scores[NUM_SCORES];
    bool score_used[NUM_SCORES + 1] = {false, false, false, false, false, false,
                                       false, false, false, false, false};

    for (uint8_t next_score_index = 0; next_score_index < NUM_SCORES; ++next_score_index) {
      // For all scores
        uint8_t highest_score_index = 0;
        uint32_t highest_score = 0;
        for (uint8_t i = 0; i <= NUM_SCORES; ++i) {
          // For all the number of scores
            if (!score_used[i]) {
              // when the score is not used
                if (i == NUM_SCORES) {
                  // Score has reached max
                    if (score->score > highest_score) {
                        highest_score = score->score;
                        highest_score_index = i;
                    }
                } else if (scoreboard[i].score >= highest_score) {
                    highest_score = scoreboard[i].score;
                    highest_score_index = i;
                }
            }
        }

        if (highest_score_index == NUM_SCORES) {
          // If we found the highest, copy to the new scores index
            memcpy((void *)&(new_scores[next_score_index]), (void *)score,
                   sizeof(struct highscore_t));
        } else {
            memcpy((void *)&(new_scores[next_score_index]),
                   (void *)&(scoreboard[highest_score_index]), sizeof(struct highscore_t));
        }
        score_used[highest_score_index] = true;
    }
    memcpy((void *)&scoreboard, (void *)&new_scores, FILE_SIZE);
}

// Loads all the highscores in file into the display
void load_highscores() {
    if (access(FILE_NAME, F_OK) == FILE_DNE) {
      // If we can't open the file bc it doesn't exist we make a new one
        printf("initing new file\n");
        for (uint8_t i = 0; i < NUM_SCORES; ++i) {
          // Print initials for all of the socers
            scoreboard[i].name[LETTER1] = 'A';
            scoreboard[i].name[LETTER2] = 'A';
            scoreboard[i].name[LETTER3] = 'A';
            scoreboard[i].score = 0;
        }
    } else {
      // file exists and we read contents into our highscore struct
        int highscores_file = open(FILE_NAME, O_RDWR, S_IRUSR | S_IRGRP | S_IROTH);
        read(highscores_file, scoreboard, sizeof(scoreboard));
        close(highscores_file);
    }
}

// Saves the higscores inside the highscore struct to a file
void save_highscores() {
  // Open the highscores file and writes
    int highscores_file = open(FILE_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    write(highscores_file, scoreboard, sizeof(scoreboard));
    close(highscores_file);
}

// clang-format off
// Takes the highscore struct and displays the top values
void display_highscore_table() {
  printf("\n");
  for (uint8_t i = 0; i < NUM_SCORES; ++i) {
    // prints all the scoers
    char name[] = {scoreboard[i].name[LETTER1], scoreboard[i].name[LETTER2], scoreboard[i].name[LETTER3], '\0'};
    hdmi_print_string((char*)name, sizeof(name), INITIALS_SCALE, white, black, INITIAL_OFFSET + i * INITIAL_SCALE_WID, INITIAL_FACTOR_1);
    
    char score[MAX_SCORE_SIZE];
    sprintf(score, "%d", scoreboard[i].score);
    hdmi_print_string(score, strlen(score), INITIALS_SCALE, white, black, INITIAL_OFFSET + i * INITIAL_SCALE_WID, INITIAL_FACTOR_2);

    printf("%c%c%c            %u\n",
           scoreboard[i].name[LETTER1],
           scoreboard[i].name[LETTER2],
           scoreboard[i].name[LETTER3],
           scoreboard[i].score);
  }
}
//clang-format on

// Button logic to update name on hs
void update_name() {
  uint32_t buttons = button_update(buttons_read());

  // When btn0 pressed basic rollover routine
  if (buttons & BUTTONS_0_MASK) {
    switch (selected_letter) {
    case LETTER1:
      if (letter1 == 'Z') {
        letter1 = 'A';
      } else {
        ++letter1;
      }
      break;
    case LETTER2:
      if (letter2 == 'Z') {
        letter2 = 'A';
      } else {
        ++letter2;
      }
      break;
    case LETTER3:
      if (letter3 == 'Z') {
        letter3 = 'A';
      } else {
        ++letter3;
      }
      break;
    default:
      break;
    }
  }
    // When btn1 pressed basic rollover routine 
  else if (buttons & BUTTONS_1_MASK) {
    switch (selected_letter) {
    case LETTER1:
      if (letter1 == 'A') {
        letter1 = 'Z';
      } else {
        --letter1;
      }
      break;
    case LETTER2:
      if (letter2 == 'A') {
        letter2 = 'Z';
      } else {
        --letter2;
      }
      break;
    case LETTER3:
      if (letter3 == 'A') {
        letter3 = 'Z';
      } else {
        --letter3;
      }
      break;
    default:
      break;
    }
  }
  // When btn2 pressed basic rollover routine  
  else if (buttons & BUTTONS_2_MASK) {
    switch (selected_letter) {
    case LETTER1:
      selected_letter = LETTER2;
      break;
    case LETTER2:
      selected_letter = LETTER3;
      break;
    case LETTER3:
      selected_letter = LETTER1;
      break;
    default:
      break;
    }
  } else if (buttons & BUTTONS_3_MASK) {
    switch (selected_letter) {
    case LETTER1:
      selected_letter = LETTER3;
      break;
    case LETTER2:
      selected_letter = LETTER1;
      break;
    case LETTER3:
      selected_letter = LETTER2;
      break;
    default:
      break;
    }
  }
  printf("\b\b\b");
  printf("%c%c%c", letter1, letter2, letter3);
  fflush(stdout);
}

// tick fucntion that controls blinking cursor and letter changes
void name_tick() {
  static uint32_t cnt = 0;
  static bool blinker = false;

  // Controls duration of blink
  if (cnt < BLINK_LENGTH) {
    ++cnt;
  }
  else {
    cnt = 0;
    blinker = !blinker;  // Toggles blinked state
    char name[INITIALS_FIELD_SIZE] = {letter1, letter2, letter3, '\0'};
    if (blinker) {
      // First style of blink
      name[LETTER1] = letter1;
      name[LETTER2] = letter2;
      name[LETTER3] = letter3;
      printf("\b\b\b");
      printf("%c%c%c", letter1, letter2, letter3);
    }
    else {
      name[selected_letter] = ' ';
      switch (selected_letter) {
      case LETTER1:
        printf("\b\b\b");
        printf(" %c%c", letter2, letter3);
      break;
      case LETTER2:
        printf("\b\b\b");
        printf("%c %c", letter1, letter3);
      break;
      case LETTER3:
        printf("\b\b\b");
        printf("%c%c ", letter1, letter2);
      break;
      }
    }
    display_name(name);
    fflush(stdout);
  }
}

// displays the game over screen. Blocking call
void display_game_over(uint32_t score) {
  set_display();
  while (true) {
    // Call interrupt controller function to wait for interrupt
    uint32_t interrupts = intc_wait_for_interrupt();
    switches_ack_interrupt();

    if (interrupts & SYSTEM_INTC_IRQ_FIT_MASK) {
      button_debouncer_tick();
      name_tick();
    }
    if (interrupts & SYSTEM_INTC_IRQ_BUTTONS_MASK) {
      buttons_ack_interrupt();
      update_name();
    }
    if (interrupts & SYSTEM_INTC_IRQ_SWITCHES_MASK) {
      switches_ack_interrupt();
      if (switches_read() & (SWITCHES_0_MASK | SWITCHES_1_MASK))
      break;
    }

    // Acknowledge the intc interrupt
    intc_ack_interrupt(interrupts);

    // Re-enable interrupts with the UIO
    intc_enable_uio_interrupts();
  }
  struct highscore_t new_score = {{letter1, letter2, letter3}, score};
  hdmi_fill_screen(black);
  char *message = "GAME OVER";
  hdmi_print_string(message, strlen(message), GO_TITLE_SCALE, white, black, GO_Y_POS, GO_X_POS);
  load_highscores();
  update_highscores(&new_score);
  save_highscores();
  display_highscore_table();
  hdmi_exit();
}
