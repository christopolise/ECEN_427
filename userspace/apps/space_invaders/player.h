#include "button_debouncer.h"
#include "hdmi.h"

// This function sets up all the preliminary variables to make sure the state machine is in
// the right condition to start
void player_init();

// Carries out the needed Mealy and Moore actions based off the given flags and states
void player_tick();
