#include <stdint.h>
#include <stdio.h>
#include "button_debouncer.h"
#include "hdmi.h"
#include "game_over.h"
#include "globals.h"
#include "world.h"
#include "player.h"
#include "alien_march.h"
#include "bunkers.h"
#include "flying_alien.h"
#include <buttons/buttons.h>
#include <intc/intc.h>
#include <switches/switches.h>
#include <system.h>
#include <stdlib.h>
#include <time.h>
#include "bullet.h"

#define EXIT_ERROR -1
#define LOAD_SUCCESS 0

// initialize everything
// returns true on success, false on failure
bool init() {
  bool success = true;

  // Initialize interrupt controller driver
  success = (intc_init(SYSTEM_INTC_UIO_FILE) == LOAD_SUCCESS);
  if (!success) {
    printf("intc_init failed\n");
    return false;
  }

  // Initialize buttons
  success = (buttons_init(SYSTEM_BUTTONS_UIO_FILE) == LOAD_SUCCESS);
  if (!success) {
    printf("buttons_init failed\n");
    return false;
  }

  // Initialize switches
  success = (switches_init(SYSTEM_SWITCHES_UIO_FILE) == LOAD_SUCCESS);
  if (!success) {
    printf("switches_init failed\n");
    return false;
  }

  // Initialize hdmi driver
  success = (hdmi_init("/dev/ecen427_hdmi") == LOAD_SUCCESS);
  if (!success) {
    printf("hdmi_init failed\n");
    return false;
  }

  globals_init(); // init global SM
  world_init(); // init world
  bullet_init(); // init bullets
  alien_march_init(); // init alien march
  bunkers_init(); // init bunkers
  player_init(); //init player

  // Enable interrupt output from buttons
  buttons_enable_interrupts();

  // Enable button and FIT interrupt lines on interrupt controller
  intc_irq_enable(SYSTEM_INTC_IRQ_BUTTONS_IRQ | SYSTEM_INTC_IRQ_FIT_IRQ);
  return success;
}

int main() {

  bool success = init(); // init everything
  if (!success) {
    return EXIT_ERROR;
  }
  // seed our randomness
  srand(time(NULL));

  // main event loop
  while(!globals_get_game_over())
  {
     // Call interrupt controller function to wait for interrupt
    uint32_t interrupts = intc_wait_for_interrupt();
    switches_ack_interrupt();

    //true every 10 ms
    if (interrupts & SYSTEM_INTC_IRQ_FIT_MASK) {
      button_debouncer_tick();
      player_tick();
      globals_tick();
      bullet_tick();
      alien_march_tick();
      flying_alien_tick();
      bunkers_tick();
    }
    // do nothing for switches
    if (interrupts & SYSTEM_INTC_IRQ_BUTTONS_MASK) {
      buttons_ack_interrupt();
    }

    // Acknowledge the intc interrupt
    intc_ack_interrupt(interrupts);

    // Re-enable interrupts with the UIO
    intc_enable_uio_interrupts();
  }

  // move to the game over screen
  display_game_over(globals_getScore());

  // clean up
  intc_exit();
  buttons_exit();
  switches_exit();
  

  return 0;
}
