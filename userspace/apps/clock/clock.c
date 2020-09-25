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
#define CLR_SCREEN "\x1B[1;1H\x1B[2J" // Regex to clear a screen

// Counters that help with keeping track of timesensitive increments and rapid
// fire
static uint32_t time_count = 0;
static uint32_t debounce_cnt = 0;
static uint32_t rapid_fire = 0;
static uint32_t rapid_fire_delay = 0;
static bool btnup = true;
static bool rapid_fire_enable = false;

// Meant to facilitate switch statements that require a time place value
enum timeplace { SEC, MIN, HOUR } placeval;

// Time place values that will be used for incremental purposes
static int8_t hours = HOURS_START;
static int8_t minutes = MINUTES_START;
static int8_t seconds = SECONDS_START;

// Button global variables to help with debouncing and action to be taken
static uint8_t buttons;
static uint8_t buttons_old;

// Strings that will hold time place valie
char sec[TIME_PLACEHOLDER_SIZE] = {"na"};
char min[TIME_PLACEHOLDER_SIZE] = {"na"};
char hrs[TIME_PLACEHOLDER_SIZE] = {"na"};

// Polices the format of the current hours, minutes, and seconds value, ensures
// to prepend extra 0 to keep HH:MM:SS format
// @param int8_t timeval: input desiresd time place value to check if a
// prepended zero is needed
// @param char printval[]: string where the correct placevalue format will be
// stored
void prepend_zero(int8_t timeval, char printval[TIME_PLACEHOLDER_SIZE]) {

  if (!(timeval / DOUBLE_DIGIT_NUM_TEST)) {
    // If the number is not a two digit number, prepend a 0 to beginning
    snprintf(printval, sizeof(char[TIME_PLACEHOLDER_SIZE]), "0%u", timeval);
  } else {
    // The number needs no prepended zero, it is passed to appropriate string
    snprintf(printval, sizeof(char[TIME_PLACEHOLDER_SIZE]), "%u", timeval);
  }
}

// @param bool inc: boolean that dictates whether you increment (T) or decrement
// (F)
// @param uint8_t val:receives a parameter from enum to determine whether SECS,
// MINS, or HOURS will be incremented/decremented
// User indicates whether to increment/decrement and which value to perform said
// action
void incdec_timeval(bool inc, uint8_t val) {
  switch (val) {
    // Chooses time place value to increment or decrement (@inc) based on value
    // of @val
  case SEC:
    inc ? seconds++ : seconds--;
    if (seconds == SECONDS_UNDERFLOW) {
      // Decrementing when the sec val is already 0, resets to 59
      seconds = SECONDS_MAX;
    } else if (seconds == SECONDS_OVERFLOW) {
      // Incrementing when the sec val is already 59, resets to 0
      seconds = TIMEVAL_MIN;
    }
    prepend_zero(seconds, sec);
    break;
  case MIN:
    inc ? minutes++ : minutes--;
    if (minutes == MINUTES_UNDERFLOW) {
      // Decrementing when the min val is already 0, resets to 59
      minutes = MINUTES_MAX;
    } else if (minutes == MINUTES_OVERFLOW) {
      // Incrementing when the min val is already 59, resets to 0
      minutes = TIMEVAL_MIN;
    }
    prepend_zero(minutes, min);
    break;
  case HOUR:
    inc ? hours++ : hours--;
    if (hours == HOURS_UNDERFLOW) {
      // Decrementing when the hour val is already 0, resets to 23
      hours = HOURS_MAX;
    } else if (hours == HOURS_OVERFLOW) {
      // Incrementing when the hour val is already 23, resets to 0
      hours = TIMEVAL_MIN;
    }
    prepend_zero(hours, hrs);
    break;
  }
}

// Routine that ensures a steady incrementing of time with the appropriate
// rollovers and formatting every second it is called
void increment_time() {
  if (hrs[TIMESTART_IND] == 'n') {
    // The time has been properly initialized and can now format the time place
    // values to their correct format
    prepend_zero(seconds, sec);
    prepend_zero(minutes, min);
    prepend_zero(hours, hrs);
  }
  incdec_timeval(true, SEC);
  if (seconds == TIMEVAL_MIN) {
    // Second rollover incrementing the minute place value
    incdec_timeval(true, MIN);
    if (minutes == TIMEVAL_MIN) {
      // Minute rollover incrementing the hour place value
      incdec_timeval(true, HOUR);
    }
  }
}

// Logic to execute the appropriate tasks as per button debounced button press
// Jumps through switch to query switches and execute desired inc/dec action
void run_buttons() {

  if (buttons == BUTTONS_0_MASK || buttons == BUTTONS_1_MASK ||
      buttons == BUTTONS_2_MASK || buttons == BUTTONS_3_MASK) {
    // Only one of the buttons has been pressed
    switch (buttons) {
      // Chooses to execute an increment or decrement based on SW0 position and
      // the place value to been modified as indicated by enum member
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
      // Nothing to see here
      break;
    }
  }
}

// This is invoked in response to a timer interrupt.
// It does 2 things: 1) help debounce buttons, and 2) advances the time.
// Logic for rapid fire is also included in this function
// Increments timers, then executes subroutines as the if statements denote,
// runs every 10 ms
void isr_fit() {
  time_count++;   // Increment one second time counter
  debounce_cnt++; // Increment debounce delay counter

  if (!btnup) {
    // Button is press and the half-second counter is started to enable rapid
    // fire
    rapid_fire_delay++;
  } else {
    // Button is not pressed therefore rapid fire is delayed
    rapid_fire_delay = COUNTER_RESET;
    rapid_fire_enable = false;
  }

  if (rapid_fire_delay == HALF_SEC_DELAY) {
    // Half a second has passed with a continuous button press
    rapid_fire_enable = true;
  }

  if (rapid_fire_enable) {
    // Half a second has passed with a continuous button press
    // Start timer to increment time val by 10 per second
    rapid_fire++;
    if (rapid_fire == RAPID_INCDEC_DELAY) {
      // One-tenth of a second has passed enabling a rapid fire inc/dec
      rapid_fire = COUNTER_RESET;
      run_buttons();
    }
  } else {
    // Reset rapid fire shooting timer
    rapid_fire = COUNTER_RESET;
  }

  if (debounce_cnt == DEBOUNCE_DELAY) {
    // Button successfully debounced
    if (buttons != buttons_old)
      // Prevents accidental double presses and toggles state of button
      btnup = !btnup;
    run_buttons();
    buttons_old = buttons;
  }

  if (time_count == SYSTEM_FIT_HZ_IN_MS * ONE_SEC_FACTOR) {
    // One second has passed which means we need to increment clock
    if (!((switches_read() & SWITCHES_ALL_MASK) & SWITCHES_1_MASK) && btnup) {
      // Button is not pressed and the pause switch (SW1) is not up so we can
      // increment time
      increment_time();
    }
    // Restart one second counter
    time_count = COUNTER_RESET;
  }

  // Clear screen and print formatted time to it
  printf(CLR_SCREEN);
  printf("%s:%s:%s\n\r", hrs, min, sec);
}

// This is invoked each time there is a change in the button state (result of
// a push or a bounce).
void isr_buttons() {
  // Read the state of the buttons
  buttons = buttons_read() & BUTTONS_ALL_MASK;
  debounce_cnt = COUNTER_RESET;
  // Acknowledge the button interrupt
  buttons_ack_interrupt();
}

// Loads all drivers for buttons, switches, and intc
// Loops forever in time telling program
// returns zero as per convention, should never be reached
int main() {

  int32_t err = LOAD_SUCCESS;

  // Initialize interrupt controller driver
  err = intc_init(SYSTEM_INTC_UIO_FILE);
  if (err) {
    // Exits program with error because the initialization of driver failed
    printf("intc_init failed\n");
    exit(EXIT_ERROR);
  }

  // Initialize buttons
  err = buttons_init(SYSTEM_BUTTONS_UIO_FILE);
  if (err) {
    // Exits program with error because the initialization of driver failed
    printf("buttons_init failed\n");
    exit(EXIT_ERROR);
  }

  // Initialize switches
  err = switches_init(SYSTEM_SWITCHES_UIO_FILE);
  if (err) {
    // Exits program with error because the initialization of driver failed
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
