#include "world.h"
#include "hdmi.h"
#include "sounds.h"
#include "sprites.h"
#include <string.h>

#define GRASS_OFFSET 5
#define LABEL_SIZE 6
#define LABEL_SCALE 2
#define SCORE_LABEL_Y_POS 10
#define SCORE_LABEL_X_POS 10
#define LIVES_LABEL_Y_POS 10
#define LIVES_LABEL_X_OFFSET 200
#define TANK_X 17
#define TANK_Y 8
#define LIVES_SCALE 1
#define LIFE_1 135
#define LIFE_2 115
#define LIFE_3 95
#define SCORE_X_POS 75

char background[HDMI_COLOR_FACTOR] = {0x00, 0x00, 0x00};
char label_color[HDMI_COLOR_FACTOR] = {0xFF, 0xFF, 0xFF};
char grass[HDMI_COLOR_FACTOR] = {0x00, 0xFF, 0x00};
char tank_color[HDMI_COLOR_FACTOR] = {0x78, 0xB9, 0xBF};

// Responsible for drawing world background
void world_init() {
    sounds_init(SOUNDS_DEVICE_FILE);
    hdmi_init(HDMI_DEVICE_FILE);
    hdmi_fill_screen(background);
    hdmi_draw_row(HDMI_DISPLAY_HEIGHT - GRASS_OFFSET, grass);
    char score[LABEL_SIZE] = "score\0";
    char lives[LABEL_SIZE] = "lives\0";
    hdmi_print_string(score, strlen(score), LABEL_SCALE, label_color, background, SCORE_LABEL_Y_POS, 
                      SCORE_LABEL_X_POS);
    hdmi_print_string(lives, strlen(lives), LABEL_SCALE, label_color, background, LIVES_LABEL_Y_POS,
                      HDMI_DISPLAY_WIDTH - LIVES_LABEL_X_OFFSET);

    char temp_score[LABEL_SIZE] = {"00000\0"};
    hdmi_print_string(temp_score, strlen(temp_score), LABEL_SCALE, grass, background, SCORE_LABEL_Y_POS, SCORE_X_POS);
    hdmi_draw_sprite(sprite_tank_17x8, TANK_X, TANK_Y, LIVES_SCALE, tank_color, background, SCORE_LABEL_Y_POS,
                     HDMI_DISPLAY_WIDTH - LIFE_1);
    hdmi_draw_sprite(sprite_tank_17x8, TANK_X, TANK_Y, LIVES_SCALE, tank_color, background, SCORE_LABEL_Y_POS,
                     HDMI_DISPLAY_WIDTH - LIFE_2);
    hdmi_draw_sprite(sprite_tank_17x8, TANK_X, TANK_Y, LIVES_SCALE, tank_color, background, SCORE_LABEL_Y_POS,
                     HDMI_DISPLAY_WIDTH - LIFE_3);
}
