#include "hdmi.h"
#include "sprites.h"
#include <ctype.h>

#define HDMI_OPEN_ERROR -1
#define HDMI_CLOSE_ERROR -1
#define HDMI_ERROR -1
#define HDMI_SUCCESS 0
#define LSEEK_ERROR -1
#define WRITE_ERROR -1
#define READ_ERROR -1
#define WRITE_SUCCESS 0
#define SCALE_ERROR -1
#define LETTER_DIM 5
#define FIRST_COL 0
#define MIDDLE_COL 1
#define LAST_COL 2
#define STRING_KERNING 6
#define ENABLE_PIX 1
#define DISABLE_PIX 0
#define HDMI_INIT_ERR_MSG "hdmi init error -- did you forget to sudo?\n"
#define LSEEK_ERR_MSG "lseek has encountered an error\n"
#define READ_ERR_MSG "Reading error encountered\n"
#define FILE_ERR_MSG "File close error encountered\n"

// File descriptor for hdmi device
static int fd;

// Takes the given sprite line and scales it up correctly to facilitate
// line-scaled sprites for faster writing
// @param - width uint8_t that denotes the sprite width
// @param - scale uint8_t that scales the sprite
// @param - buf_in uint8_t array that denotes which bytes to draw or not
// @param - buf_out array that populates the array to be written
// @param - foreground_color char * that denotes foreground color of sprite
// @param - background_color char * that denotes background_color color of
// sprite
void create_sprite_write_line_buffer(uint8_t width, uint8_t scale,
                                     uint8_t *buf_in, char *buf_out,
                                     char foreground_color[HDMI_COLOR_FACTOR],
                                     char background_color[HDMI_COLOR_FACTOR]) {
  uint16_t newind = 0;
  for (uint8_t i = 0; i < width; i++) {
    // Loop through columns
    for (int8_t j = 0; j < scale; j++) {
      // Repeat for number of scaled times
      // foreground_colored block
      if (buf_in[i] == 1) {
        for (uint8_t k = 0; k < HDMI_COLOR_FACTOR; k++) {
          // Fill in all the color pixels
          buf_out[newind++] = foreground_color[k];
        }
        // background_colored block
      } else if (buf_in[i] == 0) {
        for (uint8_t k = 0; k < HDMI_COLOR_FACTOR; k++) {
          // Fill in all the color pixels
          buf_out[newind++] = background_color[k];
        }
      }
    }
  }
  // Null terminate output buffer
  buf_out[newind] = '\0';
}

// Prints, specifically, a character to the screen at a desired offset with a
// desired color and applied scaling
// @param: letter - char that is the letter to be printed
// @param: scale - uint8_t that provides scaling factor to letter
// @param: foreground_color - 3 byte array that applies color to character drawn
// @param: x_offset - uint32_t that shows where the horizontal position of
// letter starts
// @param: y_offset - uint32_T that shows where the vertical position of the
// letter starts
void print_char(char letter, uint8_t scale,
                char foreground_color[HDMI_COLOR_FACTOR],
                char background_color[HDMI_COLOR_FACTOR], uint32_t x_offset,
                uint32_t y_offset) {
  switch (tolower(letter)) {
    // a lower case version of letter is checked against all possible drawable
    // characters and is then drawn on the screen with the appropriate color and
    // scale
  case 'a':
    hdmi_draw_sprite(sprite_letterA_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'b':
    hdmi_draw_sprite(sprite_letterB_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'c':
    hdmi_draw_sprite(sprite_letterC_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'd':
    hdmi_draw_sprite(sprite_letterD_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'e':
    hdmi_draw_sprite(sprite_letterE_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'f':
    hdmi_draw_sprite(sprite_letterF_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'g':
    hdmi_draw_sprite(sprite_letterG_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'h':
    hdmi_draw_sprite(sprite_letterH_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'i':
    hdmi_draw_sprite(sprite_letterI_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'j':
    hdmi_draw_sprite(sprite_letterJ_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'k':
    hdmi_draw_sprite(sprite_letterK_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'l':
    hdmi_draw_sprite(sprite_letterL_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'm':
    hdmi_draw_sprite(sprite_letterM_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'n':
    hdmi_draw_sprite(sprite_letterN_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'o':
    hdmi_draw_sprite(sprite_letterO_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'p':
    hdmi_draw_sprite(sprite_letterP_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'q':
    hdmi_draw_sprite(sprite_letterQ_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'r':
    hdmi_draw_sprite(sprite_letterR_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 's':
    hdmi_draw_sprite(sprite_letterS_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 't':
    hdmi_draw_sprite(sprite_letterT_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'u':
    hdmi_draw_sprite(sprite_letterU_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'v':
    hdmi_draw_sprite(sprite_letterV_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'w':
    hdmi_draw_sprite(sprite_letterW_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'x':
    hdmi_draw_sprite(sprite_letterX_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'y':
    hdmi_draw_sprite(sprite_letterY_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case 'z':
    hdmi_draw_sprite(sprite_letterZ_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case '0':
    hdmi_draw_sprite(sprite_number0_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case '1':
    hdmi_draw_sprite(sprite_number1_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case '2':
    hdmi_draw_sprite(sprite_number2_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case '3':
    hdmi_draw_sprite(sprite_number3_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case '4':
    hdmi_draw_sprite(sprite_number4_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case '5':
    hdmi_draw_sprite(sprite_number5_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case '6':
    hdmi_draw_sprite(sprite_number6_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case '7':
    hdmi_draw_sprite(sprite_number7_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case '8':
    hdmi_draw_sprite(sprite_number8_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case '9':
    hdmi_draw_sprite(sprite_number9_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  case ' ':
    hdmi_draw_sprite(sprite_letterBLANK_5x5, LETTER_DIM, LETTER_DIM, scale,
                     foreground_color, background_color, x_offset, y_offset);
    break;
  }
}

// Draws a scaled pixel at the indicated x and y positions with the desired
// color. Writes to HDMI file
// @param: pos_x - uint32_t that describes the x offset from the origin
// (top-left corner)
// @param: pos_y - uint32_t that describes the y offset from the origin
// (top-left corner)
// @param: color - 3 byte char array that uses RGB values to give the pixel the
// desired color
// @param: scale - uint8_t that provides a scaling factor to the drawn pixel
void draw_pixel(uint32_t pos_x, uint32_t pos_y, char color[HDMI_COLOR_FACTOR],
                uint8_t scale) {

  char color_pix[HDMI_COLOR_FACTOR * scale];
  for (int i = 0; i < HDMI_COLOR_FACTOR * scale; i += HDMI_COLOR_FACTOR) {
    // Loops through scaled color array and populates expanded array with all
    // the same values as to color larger areas of the screen faster
    color_pix[i] = color[FIRST_COL];
    color_pix[i + MIDDLE_COL] = color[MIDDLE_COL];
    color_pix[i + LAST_COL] = color[LAST_COL];
  }

  for (uint16_t i = 0; i < scale; i++) {
    // Logic for scaling, we lseek to the correct location based off the given
    // offset, write the x dimension of the bit and then calculates the next
    // line down's x location and repeats the same draw
    // HDMI_DISPLAY_WIDTH * HDMI_COLOR_FACTORis summed with the offset to skip
    // to the next line
    lseek(fd,
          (pos_x * HDMI_COLOR_FACTOR) +
              (pos_y * HDMI_COLOR_FACTOR * HDMI_DISPLAY_WIDTH) +
              (i * HDMI_DISPLAY_WIDTH * HDMI_COLOR_FACTOR),
          SEEK_SET);
    write(fd, color_pix, HDMI_COLOR_FACTOR * scale);
  }
}

// Prints desired string to screen with correct scale and color
// Uses print_sprite and print_char to complete this
// @param: mesg - string that will be printed to screen
// @param: str_size - uint16_t that determines how far we can iterate safely
// @param: scale - uint8_t determines the size of the printed string
// @param: foreground_color - 3 byte char array that determines string's color
// @param: x_offset - uint32_t horizontal offset for string
// @param: y_offset - uint32_t vertical offset for string
void hdmi_print_string(char *mesg, uint16_t str_size, uint8_t scale,
                       char foreground_color[HDMI_COLOR_FACTOR],
                       char background_color[HDMI_COLOR_FACTOR],
                       uint32_t y_offset, uint32_t x_offset) {

  for (uint16_t i = 0; i < str_size; i++) {
    // For the amount of the characters in the string, print each character on
    // the screen in the desired offset and with the desired scale and color
    print_char(mesg[i], scale, foreground_color, background_color, y_offset,
               x_offset + (i * STRING_KERNING * scale));
  }
}

// Draws the desired sprite on the screen with the correct scale and color
// Uses draw pixel and grid logic to print correct pixels in the grid
// @param: sprite_name - uint32_t array that uses the sprite info to draw
// @param: width - uint8_t that denotes the width of the sprite
// @param: height - uint8_t that denotes the height of the sprite
// @param: foreground_color - 3 byte char array that applies color to sprite
// @param: y_offset - uint32_t that denotes the y position on screen
// @param: x_offset - uint32_t that denotes the x position on screen
void hdmi_draw_sprite(const uint32_t *sprite_name, uint8_t width,
                      uint8_t height, uint8_t scale,
                      char foreground_color[HDMI_COLOR_FACTOR],
                      char background_color[HDMI_COLOR_FACTOR],
                      uint32_t y_offset, uint32_t x_offset) {

  uint32_t row, column;
  char row_data[HDMI_COLOR_FACTOR * width * scale];
  uint8_t sprite_line[width];

  for (row = 0; row < height; row++) {
    // For each row, print out the necessary pixels for the sprite

    for (column = 0; column < width; column++) {
      // For each column, print out the necessary pixels for the sprite

      if ((sprite_name[row] & (1 << (width - 1 - column)))) {
        // If the pixmap denotes that there is a pixel to be drawn, do it and
        // apply correct color and scale
        sprite_line[column] = ENABLE_PIX;
      } else {
        sprite_line[column] = DISABLE_PIX;
      }
    }

    // Creates scaled line to write to screen based off of sprite info
    create_sprite_write_line_buffer(width, scale, sprite_line, row_data,
                                    foreground_color, background_color);

    // Repeats the line drawn for the number of scale as to scale vertically
    for (int i = 0; i < scale; i++) {
      lseek(fd,
            (x_offset * HDMI_COLOR_FACTOR) +
                (y_offset * HDMI_COLOR_FACTOR * HDMI_DISPLAY_WIDTH),
            SEEK_SET);
      write(fd, row_data, HDMI_COLOR_FACTOR * scale * width);
      y_offset++;
    }
  }
}

// Draws a row on the screen using one screen-width'd buffer
// Used in fill screen to accelerate filling process
// @param: pos_y - uint32_t that describes the y offset of the row to be changed
// @param: color - 3 byte char array to provide an RGB color for the row
void hdmi_draw_row(uint32_t pos_y, char color[HDMI_COLOR_FACTOR]) {
  char color_line[HDMI_DISPLAY_WIDTH * HDMI_COLOR_FACTOR];
  for (uint16_t i = 0; i < (HDMI_DISPLAY_WIDTH * HDMI_COLOR_FACTOR);
       i += HDMI_COLOR_FACTOR) {
    // Expands the color pattern into the array that will write to the row
    color_line[i] = color[FIRST_COL];
    color_line[i + MIDDLE_COL] = color[MIDDLE_COL];
    color_line[i + LAST_COL] = color[LAST_COL];
  }
  // Jump to the row to be written to and write
  lseek(fd, HDMI_DISPLAY_WIDTH * pos_y * HDMI_COLOR_FACTOR, SEEK_SET);
  write(fd, color_line, HDMI_DISPLAY_WIDTH * HDMI_COLOR_FACTOR);
}

// Fills the entire display with a certain color by calling draw_row function
// @param: color - 3 byte char array to provide an RGB color for the screen
void hdmi_fill_screen(char color[HDMI_COLOR_FACTOR]) {
  for (int i = 0; i < HDMI_DISPLAY_HEIGHT; i++)
    // Loops through all row of the screen and changes their color
    hdmi_draw_row(i, color);
}

// Initializes the HDMI device file. Checks for a successful opening of
// "/dev/ecen427_hdmi"
// @param: devFile - char * that is thename of the device file where the HDMI
// device is represented
// RETURNS: HDMI_SUCCESS(0) if the file opens successfully, HDMI_OPEN_ERROR(-1)
// if the file doesn't initialize correctly
int32_t hdmi_init(char *devFile) {
  fd = open(devFile, O_RDWR);
  if (fd == HDMI_OPEN_ERROR) {
    // Upon error returned by open, we exit function with similar error and
    // print out a msg to user
    printf(HDMI_INIT_ERR_MSG);
    return HDMI_OPEN_ERROR;
  }

  return HDMI_SUCCESS;
}

// Reads pixel values from certain location in the HDMI file.
// @param: char_size - size_t that represents the desired amount of bytes to be
// read
// @param: offset - uint32_t that represents the value to start reading from
// RETURNS: String that contains information from read bytes
char *hdmi_read(size_t char_size, uint32_t offset) {
  char *readval = malloc(sizeof(char) * char_size + 1);

  if (lseek(fd, offset, SEEK_SET) == LSEEK_ERROR) {
    // If the lseek function cannot iterate to the correct location
    // Exits with error code to user
    printf(LSEEK_ERR_MSG);
    exit(LSEEK_ERROR);
  }

  int status = read(fd, readval, char_size);
  if (status == READ_ERROR) {
    // If reading the buffer is invalid or cannot happen
    // Exits with a read error code and prints info to terminal
    printf(READ_ERR_MSG);
    exit(READ_ERROR);
  }

  // Null terminate info read
  readval[status] = '\0';

  return readval;
}

// Closes device file for the HDMI controller
void hdmi_exit() {
  if (close(fd) == HDMI_CLOSE_ERROR) {
    // If there is an issue closing the hdmi device file
    // Prints error message and exits with error code
    printf(FILE_ERR_MSG);
    exit(HDMI_CLOSE_ERROR);
  }
}
