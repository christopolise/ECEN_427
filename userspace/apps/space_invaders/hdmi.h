#ifndef __HDMI_H__
#define __HDMI_H__

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define HDMI_DISPLAY_WIDTH 640
#define HDMI_DISPLAY_HEIGHT 480
#define HDMI_COLOR_FACTOR 3
#define HDMI_DEVICE_FILE "/dev/ecen427_hdmi"

// Initializes the HDMI device file. Checks for a successful opening of
// "/dev/ecen427_hdmi"
// @param: devFile - char * that is thename of the device file where the HDMI
// device is represented
// RETURNS: HDMI_SUCCESS(0) if the file opens successfully, HDMI_OPEN_ERROR(-1)
// if the file doesn't initialize correctly
int32_t hdmi_init(char *devFile);

// Reads pixel values from certain location in the HDMI file.
// @param: char_size - size_t that represents the desired amount of bytes to be
// read
// @param: offset - uint32_t that represents the value to start reading from
// RETURNS: String that contains information from read bytes
char *hdmi_read(size_t charsize, uint32_t offset);

// Fills the entire display with a certain color by calling draw_row function
// @param: color - 3 byte char array to provide an RGB color for the screen
void hdmi_fill_screen(char color[HDMI_COLOR_FACTOR]);

// clang-format off

// Draws the desired sprite on the screen with the correct scale and color
// Uses draw pixel and grid logic to print correct pixels in the grid
// @param: sprite_name - uint32_t array that uses the sprite info to draw
// @param: width - uint8_t that denotes the width of the sprite
// @param: height - uint8_t that denotes the height of the sprite
// @param: foreground_color - 3 byte char array that applies color to sprite
// @param: y_offset - uint32_t that denotes the y position on screen
// @param: x_offset - uint32_t that denotes the x position on screen
void hdmi_draw_sprite(const uint32_t *sprite_name, 
                      uint8_t width,
                      uint8_t height, 
                      uint8_t scale, 
                      char foreground_color[HDMI_COLOR_FACTOR],
                      char background_color[HDMI_COLOR_FACTOR],
                      uint32_t y_offset,
                      uint32_t x_offset);

// Prints desired string to screen with correct scale and color
// Uses print_sprite and print_char to complete this
// @param: mesg - string that will be printed to screen
// @param: str_size - uint16_t that determines how far we can iterate safely
// @param: scale - uint8_t determines the size of the printed string
// @param: foreground_color - 3 byte char array that determines string's color
// @param: x_offset - uint32_t horizontal offset for string
// @param: y_offset - uint32_t vertical offset for string
void hdmi_print_string(char *mesg, 
                       uint16_t str_size, 
                       uint8_t scale,
                       char foreground_color[HDMI_COLOR_FACTOR],
                       char background_color[HDMI_COLOR_FACTOR], 
                       uint32_t y_offset,
                       uint32_t x_offset);

// clang-format on

// Draws a row on the screen using one screen-width'd buffer
// Used in fill screen to accelerate filling process
// @param: pos_y - uint32_t that describes the y offset of the row to be changed
// @param: color - 3 byte char array to provide an RGB color for the row
void hdmi_draw_row(uint32_t pos_y, char color[HDMI_COLOR_FACTOR]);

// Closes device file for the HDMI controller
void hdmi_exit();

#endif