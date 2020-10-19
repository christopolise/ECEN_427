#include "button_debouncer.h"

#define DEBOUNCE_CYCLES 15

static uint8_t cnt = 0U;

// standard tick function
void button_debouncer_tick() {
  // debounce time expired
  if (cnt < DEBOUNCE_CYCLES) {
    ++cnt;
  }
}

// update buttons with new state
// returns a debounced version the buttons parameter
uint32_t button_update(uint32_t buttons) {
  // reset debounce time if it is maxed out
  if (cnt == DEBOUNCE_CYCLES) {
    cnt = 0;
    return buttons;
  }
  return 0U;
}