
#ifndef __BUTTON_SM_H__
#define __BUTTON_SM_H__

#include <stdint.h>

// standard tick function
void button_debouncer_tick();

// update buttons with new state
// @param[in] current button state
// @return debounced button state
uint32_t button_update(uint32_t buttons);

#endif