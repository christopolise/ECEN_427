#include "buttons/buttons.h"
#include "intc/intc.h"
#include "switches/switches.h"
#include "system.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_ERROR -1

static uint32_t count = 0;
static uint32_t hours = 12;
static uint32_t minutes = 59;
static uint32_t seconds = 45;

char sec[4];
char min[4];
char hrs[4];

void prepend_zero(uint16_t timeval, char printval[4]) {

  if (!(timeval / 10)) {
    snprintf(printval, sizeof(char[4]), "0%u", timeval);
  } else {
    snprintf(printval, sizeof(char[4]), "%u", timeval);
  }
}

void increment_time() {
  seconds++;
  if (seconds == 60) {
    seconds = 0;
    minutes++;
    if (minutes == 60) {
      minutes = 0;
      hours++;
      if (hours == 13) {
        hours = 1;
      }
    }
  }

  prepend_zero(seconds, sec);
  prepend_zero(minutes, min);
  prepend_zero(hours, hrs);
}

// This is invoked in response to a timer interrupt.
// It does 2 things: 1) help debounce buttons, and 2) advances the time.
void isr_fit() {
  //
  count++;
  if (count == SYSTEM_FIT_HZ_IN_MS * 10) {
    system("clear");
    increment_time();
    printf("%s:%s:%s\n\r", hrs, min, sec);
    count = 0;
  }
}

// This is invoked each time there is a change in the button state (result of a
// push or a bounce).
void isr_buttons() {
  // Read the state of the buttons
  // ... do something ...
  // Acknowledge the button interrupt
  uint8_t buttons;

  buttons = buttons_read() & BUTTONS_ALL_MASK;
  buttons_ack_interrupt();

  switch (buttons) {
  case BUTTONS_0_MASK:
    printf("0\n\r");
    break;
  case BUTTONS_1_MASK:
    printf("1\n\r");
    break;
  case BUTTONS_2_MASK:
    printf("2\n\r");
    break;
  case BUTTONS_3_MASK:
    printf("3\n\r");
    break;
  }
}

int main() {

  int32_t err;

  // Initialize interrupt controller driver
  err = intc_init(SYSTEM_INTC_UIO_FILE);
  if (err) {
    printf("intc_init failed\n");
    exit(EXIT_ERROR);
  }

  // Initialize buttons
  err = buttons_init(SYSTEM_BUTTONS_UIO_FILE);
  if (err) {
    printf("buttons_init failed\n");
    exit(EXIT_ERROR);
  }

  // Initialize switches
  err = switches_init(SYSTEM_SWITCHES_UIO_FILE);
  if (err) {
    printf("switches_init failed\n");
    exit(EXIT_ERROR);
  }

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
