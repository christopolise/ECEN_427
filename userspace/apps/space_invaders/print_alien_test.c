#include "buttons/buttons.h"
#include "hdmi.h"
#include "intc/intc.h"
#include "sprites.h"
#include "system.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define ALIEN_HEIGHT 16

static bool start_on = true;
static uint32_t blink_counter = 0;

static uint8_t buttons;
static uint32_t debounce_cnt = 0;

static uint8_t red = 0xFF;
static uint8_t green = 0x11;
static uint8_t blue = 0x88;

char fore[3] = {0xff, 0xff, 0xff};
char cyan[3] = {0x00, 0xff, 0xff};
char magenta[3] = {0xff, 0x00, 0xff};
char yellow[3] = {0xff, 0xff, 0x00};
char fore2[3] = {0x00, 0xff, 0x00};
char back[3] = {0x00, 0x00, 0x00};

char *message = {"space\0"};
char *message2 = {"invaders\0"};
char *startmsg = {"press any button to start\0"};

void isr_fit() {
  blink_counter++;
  debounce_cnt++;

  red += 3;
  green += 3;
  blue += 3;

  char multi[3] = {red, green, blue};

  hdmi_draw_sprite(sprite_alien_top_out_12x8, 12, 8, 5, multi, back,
                   (HDMI_DISPLAY_HEIGHT / 2) - 4 * 5,
                   (HDMI_DISPLAY_WIDTH / 2) - 6 * 5);

  if (start_on) {
    hdmi_print_string(startmsg, strlen(startmsg), 1, fore, back,
                      (HDMI_DISPLAY_HEIGHT * 8 / 9),
                      (HDMI_DISPLAY_WIDTH / 2) - (strlen(startmsg) / 2 * 6));

  } else {
    hdmi_print_string(startmsg, strlen(startmsg), 1, back, back,
                      (HDMI_DISPLAY_HEIGHT * 8 / 9),
                      (HDMI_DISPLAY_WIDTH / 2) - (strlen(startmsg) / 2 * 6));
  }

  if (blink_counter == 75) {
    blink_counter = 0;
    start_on = !start_on;
  }

  if (debounce_cnt == 4 && buttons) {
    hdmi_fill_screen(back);
    hdmi_exit();
    exit(0);
  }
}

void isr_buttons() {
  // Read the state of the buttons
  buttons = buttons_read() & BUTTONS_ALL_MASK;
  debounce_cnt = 0;
  // Acknowledge the button interrupt
  buttons_ack_interrupt();
}

int main() {

  int32_t err = 0;

  // Initialize interrupt controller driver
  err = intc_init(SYSTEM_INTC_UIO_FILE);
  if (err) {
    // Exits program with error because the initialization of driver failed
    printf("intc_init failed\n");
    exit(-1);
  }

  // Initialize buttons
  err = buttons_init(SYSTEM_BUTTONS_UIO_FILE);
  if (err) {
    // Exits program with error because the initialization of driver failed
    printf("buttons_init failed\n");
    exit(-1);
  }

  err = hdmi_init("/dev/ecen427_hdmi");
  if (err) {
    // Exits program with error because the initialization of driver failed
    printf("hdmi_init failed\n");
    exit(-1);
  }
  hdmi_fill_screen(back);
  hdmi_print_string(
      message, strlen(message), 5, magenta, back, (HDMI_DISPLAY_HEIGHT / 4) + 3 * 3,
      ((HDMI_DISPLAY_WIDTH / 2) - (strlen(message) / 2 * 5 * 7) + 1 * 3));

  hdmi_print_string(
      message, strlen(message), 5, cyan, back, (HDMI_DISPLAY_HEIGHT / 4) + 3 * 2,
      ((HDMI_DISPLAY_WIDTH / 2) - (strlen(message) / 2 * 5 * 7)) + 1 * 2);

  hdmi_print_string(
      message, strlen(message), 5, yellow, back, (HDMI_DISPLAY_HEIGHT / 4) + 3,
      ((HDMI_DISPLAY_WIDTH / 2) - (strlen(message) / 2 * 5 * 7)) + 1);

  hdmi_print_string(message, strlen(message), 5, fore,
                    (HDMI_DISPLAY_HEIGHT / 4),
                    (HDMI_DISPLAY_WIDTH / 2) - (strlen(message) / 2 * 5 * 7));

  hdmi_print_string(
      message2, strlen(message2), 5, magenta, back, 
      (HDMI_DISPLAY_HEIGHT * 3 / 4 - 25) + 3 * 3,
      ((HDMI_DISPLAY_WIDTH / 2) - (strlen(message2) / 2 * 5 * 6)) + 1 * 3);

  hdmi_print_string(
      message2, strlen(message2), 5, cyan, back,
      (HDMI_DISPLAY_HEIGHT * 3 / 4 - 25) + 3 * 2,
      ((HDMI_DISPLAY_WIDTH / 2) - (strlen(message2) / 2 * 5 * 6)) + 1 * 2);

  hdmi_print_string(
      message2, strlen(message2), 5, yellow, back, 
      (HDMI_DISPLAY_HEIGHT * 3 / 4 - 25) + 3 * 1,
      ((HDMI_DISPLAY_WIDTH / 2) - (strlen(message2) / 2 * 5 * 6)) + 1);

  hdmi_print_string(message2, strlen(message2), 5, fore, back,
                    (HDMI_DISPLAY_HEIGHT * 3 / 4 - 25),
                    (HDMI_DISPLAY_WIDTH / 2) - (strlen(message2) / 2 * 5 * 6));

  // Enable interrupt output from buttons
  buttons_enable_interrupts();

  // Enable button and FIT interrupt lines on interrupt controller
  intc_irq_enable(SYSTEM_INTC_IRQ_BUTTONS_IRQ | SYSTEM_INTC_IRQ_FIT_IRQ);

  while (1) {
    // Call interrupt controller function to wait for interrupt
    uint32_t interrupts = intc_wait_for_interrupt();

    // Check which interrupt lines are high and call the appropriate ISR
    // functions
    if (interrupts & SYSTEM_INTC_IRQ_FIT_MASK)
      isr_fit();
    if (interrupts & SYSTEM_INTC_IRQ_BUTTONS_MASK)
      isr_buttons();

    // Acknowledge the intc interrupt
    intc_ack_interrupt(interrupts);

    // Re-enable UIO interrupts
    intc_enable_uio_interrupts();
  }
}