#ifndef __BUNKERS_H__
#define __BUNKERS_H__

#include <stdbool.h>
#include <stdint.h>

enum bunker_area_t {
  TOP_1 = 0,
  TOP_2 = 1,
  TOP_3 = 2,
  TOP_4 = 3,
  MID_1 = 4,
  MID_2 = 5,
  MID_3 = 6,
  BOT_1 = 7,
  BOT_2 = 8,
  NONE = 0xFFFF
};

// initialize the bunkers
void bunkers_init();

// called when a bullet enters bunker territory
// @param[in] bunker_num the bunker number
// @param[in] bunker_area the area of the bunker
// @return true if the bunker successfully stops the bullet, false if otherwise
void bunkers_hit(uint8_t bunker_num, enum bunker_area_t bunker_area);

// standard tick function
void bunkers_tick();

#endif