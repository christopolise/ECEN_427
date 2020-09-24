#include "buttons/buttons.h"
#include "intc/intc.h"
#include "switches/switches.h"
#include "system.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_ERROR -1
#define LOAD_SUCCESS 0
#define SWITCHES_INVALID 0xFF
#define BUTTONS_INVALID 0xFF
#define TIME_PLACEHOLDER_SIZE 4
#define DOUBLE_DIGIT_NUM_TEST 10
#define HOURS_START 23
#define MINUTES_START 59
#define SECONDS_START 45
#define SECONDS_UNDERFLOW -1
#define SECONDS_OVERFLOW 60
#define MINUTES_UNDERFLOW -1
#define MINUTES_OVERFLOW 60
#define HOURS_UNDERFLOW -1
#define HOURS_OVERFLOW 24
#define TIMEVAL_MIN 0
#define SECONDS_MAX 59
#define MINUTES_MAX 59
#define HOURS_MAX 23
#define TIMESTART_IND 0
#define COUNTER_RESET 0
#define HALF_SEC_DELAY 50
#define DEBOUNCE_DELAY 4
#define ONE_SEC_FACTOR 10
#define RAPID_INCDEC_DELAY 11
#define FOREVER 1
#define CLR_SCREEN "\x1B[1;1H\x1B[2J"

static uint32_t count = 0;
static uint32_t debounce_cnt = 0;
static uint32_t rapid_fire = 0;
static uint32_t rapid_fire_delay = 0;
static bool btnup = true;
static bool rapid_fire_enable = false;

enum timeplace { SEC, MIN, HOUR } placeval;

static int8_t hours = HOURS_START;
static int8_t minutes = MINUTES_START;
static int8_t seconds = SECONDS_START;
uint8_t buttons;
uint8_t buttons_old;

char sec[TIME_PLACEHOLDER_SIZE] = {"na"};
char min[TIME_PLACEHOLDER_SIZE] = {"na"};
char hrs[TIME_PLACEHOLDER_SIZE] = {"na"};

void prepend_zero(int8_t timeval, char printval[TIME_PLACEHOLDER_SIZE]) {

  if (!(timeval / DOUBLE_DIGIT_NUM_TEST)) {
    snprintf(printval, sizeof(char[TIME_PLACEHOLDER_SIZE]), "0%u", timeval);
  } else {
    snprintf(printval, sizeof(char[TIME_PLACEHOLDER_SIZE]), "%u", timeval);
  }
}

void incdec_timeval(bool inc, uint8_t val) {
  switch (val) {
  case SEC:
    inc ? seconds++ : seconds--;
    if (seconds == SECONDS_UNDERFLOW) {
      seconds = SECONDS_MAX;
    } else if (seconds == SECONDS_OVERFLOW) {
      seconds = TIMEVAL_MIN;
    }
    prepend_zero(seconds, sec);
    break;
  case MIN:
    inc ? minutes++ : minutes--;
    if (minutes == MINUTES_UNDERFLOW) {
      minutes = MINUTES_MAX;
    } else if (minutes == MINUTES_OVERFLOW) {
      minutes = TIMEVAL_MIN;
    }
    prepend_zero(minutes, min);
    break;
  case HOUR:
    inc ? hours++ : hours--;
    if (hours == HOURS_UNDERFLOW) {
      hours = HOURS_MAX;
    } else if (hours == HOURS_OVERFLOW) {
      hours = TIMEVAL_MIN;
    }
    prepend_zero(hours, hrs);
    break;
  }
}

void increment_time() {
  if (hrs[TIMESTART_IND] == 'n') {
    prepend_zero(seconds, sec);
    prepend_zero(minutes, min);
    prepend_zero(hours, hrs);
  }
  incdec_timeval(true, SEC);
  if (seconds == TIMEVAL_MIN) {
    incdec_timeval(true, MIN);
    if (minutes == TIMEVAL_MIN) {
      incdec_timeval(true, HOUR);
    }
  }
}

void run_buttons() {

  if (buttons == BUTTONS_0_MASK || buttons == BUTTONS_1_MASK ||
      buttons == BUTTONS_2_MASK || buttons == BUTTONS_3_MASK) {
    switch (buttons) {
    case BUTTONS_0_MASK:
      incdec_timeval((switches_read() & SWITCHES_ALL_MASK) & SWITCHES_0_MASK,
                     SEC);
      break;
    case BUTTONS_1_MASK:
      incdec_timeval((switches_read() & SWITCHES_ALL_MASK) & SWITCHES_0_MASK,
                     MIN);
      break;
    case BUTTONS_2_MASK:
      incdec_timeval((switches_read() & SWITCHES_ALL_MASK) & SWITCHES_0_MASK,
                     HOUR);
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

  if (!btnup) {
    rapid_fire_delay++;
  } else {
    rapid_fire_delay = COUNTER_RESET;
    rapid_fire_enable = false;
  }

  if (rapid_fire_delay == HALF_SEC_DELAY) {
    rapid_fire_enable = true;
  }

  if (rapid_fire_enable) {
    rapid_fire++;
    if (rapid_fire == RAPID_INCDEC_DELAY) {
      rapid_fire = COUNTER_RESET;
      run_buttons();
    }
  } else {
    rapid_fire = COUNTER_RESET;
  }

  if (debounce_cnt == DEBOUNCE_DELAY) {
    if (buttons != buttons_old)
      btnup = !btnup;
    run_buttons();
    buttons_old = buttons;
  }

  if (count == SYSTEM_FIT_HZ_IN_MS * ONE_SEC_FACTOR) {

    if (!((switches_read() & SWITCHES_ALL_MASK) & SWITCHES_1_MASK) && btnup) {
      increment_time();
    }
    count = COUNTER_RESET;
  }
  printf(CLR_SCREEN);
  printf("%s:%s:%s\n\r", hrs, min, sec);
}

// This is invoked each time there is a change in the button state (result of
// a push or a bounce).
void isr_buttons() {
  // Read the state of the buttons
  // ... do something ...
  // Acknowledge the button interrupt
  buttons = buttons_read() & BUTTONS_ALL_MASK;
  debounce_cnt = COUNTER_RESET;
  buttons_ack_interrupt();
}

// Loads all drivers for buttons, switches, and intc
// Loops forever in time telling program
//
int main() {

  int32_t err = LOAD_SUCCESS;

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

  while (FOREVER) {
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

  return 0;
}
