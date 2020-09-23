#include "buttons/buttons.h"
#include "intc/intc.h"
#include "switches/switches.h"
#include "system.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_ERROR -1
#define SWITCHES_INVALID 0xFF
#define BUTTONS_INVALID 0xFF

static uint32_t count = 0;
static uint32_t debounce_cnt = 0;
static uint32_t rapid_fire = 0;
static uint32_t rapid_fire_delay = 0;
bool btnup = false;
bool rapid_fire_enable = false;
bool rapid_fire_shot = false;

enum timeplace { SEC, MIN, HOUR } placeval;

static int8_t hours = 23;
static int8_t minutes = 59;
static int8_t seconds = 45;

uint8_t switches;
uint8_t buttons;

char sec[4] = {"na"};
char min[4] = {"na"};
char hrs[4] = {"na"};

void prepend_zero(int8_t timeval, char printval[4]) {

  if (!(timeval / 10)) {
    snprintf(printval, sizeof(char[4]), "0%u", timeval);
  } else {
    snprintf(printval, sizeof(char[4]), "%u", timeval);
  }
}

void incdec_timeval(bool inc, uint8_t val) {
  switch (val) {
  case SEC:
    inc ? seconds++ : seconds--;
    if (seconds == -1) {
      seconds = 59;
    } else if (seconds == 60) {
      seconds = 0;
    }
    prepend_zero(seconds, sec);
    break;
  case MIN:
    inc ? minutes++ : minutes--;
    if (minutes == -1) {
      minutes = 59;
    } else if (minutes == 60) {
      minutes = 0;
    }
    prepend_zero(minutes, min);
    break;
  case HOUR:
    inc ? hours++ : hours--;
    if (hours == -1) {
      hours = 12;
    } else if (hours == 24) {
      hours = 0;
    }
    prepend_zero(hours, hrs);
    break;
  }
}

void increment_time() {
  if (hrs[0] == 'n') {
    prepend_zero(seconds, sec);
    prepend_zero(minutes, min);
    prepend_zero(hours, hrs);
  }
  incdec_timeval(true, SEC);
  if (seconds == 0) {
    incdec_timeval(true, MIN);
    if (minutes == 0) {
      incdec_timeval(true, HOUR);
    }
  }
}

void run_buttons() {

  if (buttons == BUTTONS_0_MASK || buttons == BUTTONS_1_MASK ||
      buttons == BUTTONS_2_MASK || buttons == BUTTONS_3_MASK) {
    switch (buttons) {
    case BUTTONS_0_MASK:
      incdec_timeval(switches & 0x1, SEC);
      break;
    case BUTTONS_1_MASK:
      incdec_timeval(switches & 0x1, MIN);
      break;
    case BUTTONS_2_MASK:
      incdec_timeval(switches & 0x1, HOUR);
      break;
    case BUTTONS_3_MASK:
      break;
    }
  }
}

// This is invoked in response to a timer interrupt.
// It does 2 things: 1) help debounce buttons, and 2) advances the time.
void isr_fit() {
  //

  count++;
  debounce_cnt++;
  switches = switches_read() & SWITCHES_ALL_MASK;

  if (!btnup) {
    rapid_fire_delay++;
  } else {
    rapid_fire_delay = 0;
    rapid_fire_enable = false;
  }

  if (rapid_fire_delay == 50) {
    rapid_fire_enable = true;
  }

  if (rapid_fire_enable) {
    rapid_fire++;
    if (rapid_fire == 11) {
      rapid_fire = 0;
      run_buttons();
    }
  } else {
    rapid_fire = 0;
  }

  if (debounce_cnt == 5) {
    btnup = !btnup;
    run_buttons();
  }

  if (count == SYSTEM_FIT_HZ_IN_MS * 10) {

    if (!(switches & SWITCHES_1_MASK)) {
      increment_time();
    }
    count = 0;
  }
  printf("\x1B[1;1H\x1B[2J");
  printf("%s:%s:%s\n\r", hrs, min, sec);
}

// This is invoked each time there is a change in the button state (result of
// a push or a bounce).
void isr_buttons() {
  // Read the state of the buttons
  // ... do something ...
  // Acknowledge the button interrupt
  buttons = buttons_read() & BUTTONS_ALL_MASK;
  debounce_cnt = 0;
  buttons_ack_interrupt();
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
