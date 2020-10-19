#include "sprites.h"

#include <stdint.h>

// Must define packWord for each of the different bit-widths.

// clang-format off

// Packs each horizontal line of the figures into a single 24 bit word. 
#define packWord24(b23,b22,b21,b20,b19,b18,b17,b16,b15,b14,b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1,b0) \
((b23 << 23) | (b22 << 22) | (b21 << 21) | (b20 << 20) | (b19 << 19) | (b18 << 18) | (b17 << 17) | (b16 << 16) |						  \
 (b15 << 15) | (b14 << 14) | (b13 << 13) | (b12 << 12) | (b11 << 11) | (b10 << 10) | (b9  << 9 ) | (b8  << 8 ) |						  \
 (b7  << 7 ) | (b6  << 6 ) | (b5  << 5 ) | (b4  << 4 ) | (b3  << 3 ) | (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )

// Packs each horizontal line of the figures into a single 16 bit word. 
#define packWord18(b17,b16,b15,b14,b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1,b0)                                                                 \
((b17 << 17) | (b16 << 16) | (b15 << 15) | (b14 << 14) | (b13 << 13) | (b12 << 12) | (b11 << 11) | (b10 << 10) | (b9  << 9 ) | (b8  << 8 ) |						  \
 (b7  << 7 ) | (b6  << 6 ) | (b5  << 5 ) | (b4  << 4 ) | (b3  << 3 ) | (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )

// Packs each horizontal line of the figures into a single 16 bit word. 
#define packWord17(b16,b15,b14,b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1,b0)                                                                 \
((b16 << 16) | (b15 << 15) | (b14 << 14) | (b13 << 13) | (b12 << 12) | (b11 << 11) | (b10 << 10) | (b9  << 9 ) | (b8  << 8 ) |						  \
 (b7  << 7 ) | (b6  << 6 ) | (b5  << 5 ) | (b4  << 4 ) | (b3  << 3 ) | (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )

// Packs each horizontal line of the figures into a single 16 bit word. 
#define packWord16(b15,b14,b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1,b0)                                                                 \
((b15 << 15) | (b14 << 14) | (b13 << 13) | (b12 << 12) | (b11 << 11) | (b10 << 10) | (b9  << 9 ) | (b8  << 8 ) |						  \
 (b7  << 7 ) | (b6  << 6 ) | (b5  << 5 ) | (b4  << 4 ) | (b3  << 3 ) | (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )

// Packs each horizontal line of the figures into a single 15 bit word. 
#define packWord15(b14,b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1,b0)                                                                     \
((b14 << 14) | (b13 << 13) | (b12 << 12) | (b11 << 11) | (b10 << 10) |                                                                    \
 (b9  << 9 ) | (b8  << 8 ) | (b7  << 7 ) | (b6  << 6 ) | (b5  << 5 ) | 																	  \
 (b4  << 4 ) | (b3  << 3 ) | (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )

 // Packs each horizontal line of the figures into a single 12 bit word. 
#define packWord12(b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1,b0)                                                                                 \
((b11 << 11) | (b10 << 10) | (b9  << 9 ) | (b8  << 8 ) |						                                                          \
 (b7  << 7 ) | (b6  << 6 ) | (b5  << 5 ) | (b4  << 4 ) |                                                                                  \
 (b3  << 3 ) | (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )


 // Packs each horizontal line of the figures into a single 6 bit word. 
#define packWord6(b5,b4,b3,b2,b1,b0)                                                                                                      \
((b5  << 5 ) | (b4  << 4 ) | (b3  << 3 ) |																						          \
 (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )

  // Packs each horizontal line of the figures into a single 6 bit word. 
#define packWord5(b4,b3,b2,b1,b0)                                                                                                         \
((b4  << 4 ) | (b3  << 3 ) | (b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )

 // Packs each horizontal line of the figures into a single 3 bit word. 
#define packWord3(b2,b1,b0)                                  		                                                                      \
((b2  << 2 ) | (b1  << 1 ) | (b0  << 0 ) )

 // Packs each horizontal line of the figures into a single 1 bit word. 
#define packWord1(b0)                                                                                                                     \
((b0  << 0 ) )


const uint32_t sprite_saucer_18x7[] =
{
	packWord18(0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0),
	packWord18(0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0),
	packWord18(0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0),
	packWord18(0,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,0),
	packWord18(0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0),
	packWord18(0,0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,0),
	packWord18(0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0)
};

const uint32_t sprite_alien_explosion_12x10[] =
{
	packWord12(0,0,0,0,0,0,1,0,0,0,0,0),
	packWord12(0,0,0,1,0,0,1,0,0,0,1,0),
	packWord12(1,0,0,1,0,0,0,0,0,1,0,0),
	packWord12(0,1,0,0,1,0,0,0,1,0,0,0),
	packWord12(0,0,0,0,0,0,0,0,0,0,1,1),
	packWord12(1,1,0,0,0,0,0,0,0,0,0,0),
	packWord12(0,0,0,1,0,0,0,1,0,0,1,0),
	packWord12(0,0,1,0,0,0,0,0,1,0,0,1),
	packWord12(0,1,0,0,0,1,0,0,1,0,0,0),
	packWord12(0,0,0,0,0,1,0,0,0,0,0,0)
};

const uint32_t sprite_alien_top_in_12x8[] =
{
	packWord12(0,0,0,0,0,1,1,0,0,0,0,0),
	packWord12(0,0,0,0,1,1,1,1,0,0,0,0),
	packWord12(0,0,0,1,1,1,1,1,1,0,0,0),
	packWord12(0,0,1,1,0,1,1,0,1,1,0,0),
	packWord12(0,0,1,1,1,1,1,1,1,1,0,0),
	packWord12(0,0,0,1,0,1,1,0,1,0,0,0),
	packWord12(0,0,1,0,0,0,0,0,0,1,0,0),
	packWord12(0,0,0,1,0,0,0,0,1,0,0,0)
};

const uint32_t sprite_alien_top_out_12x8[] =
{
	packWord12(0,0,0,0,0,1,1,0,0,0,0,0),
	packWord12(0,0,0,0,1,1,1,1,0,0,0,0),
	packWord12(0,0,0,1,1,1,1,1,1,0,0,0),
	packWord12(0,0,1,1,0,1,1,0,1,1,0,0),
	packWord12(0,0,1,1,1,1,1,1,1,1,0,0),
	packWord12(0,0,0,0,1,0,0,1,0,0,0,0),
	packWord12(0,0,0,1,0,1,1,0,1,0,0,0),
	packWord12(0,0,1,0,1,0,0,1,0,1,0,0)
};

const uint32_t sprite_alien_middle_in_12x8[] =
{
	packWord12(0,0,0,1,0,0,0,0,0,1,0,0),
	packWord12(0,0,0,0,1,0,0,0,1,0,0,0),
	packWord12(0,0,0,1,1,1,1,1,1,1,0,0),
	packWord12(0,0,1,1,0,1,1,1,0,1,1,0),
	packWord12(0,1,1,1,1,1,1,1,1,1,1,1),
	packWord12(0,1,1,1,1,1,1,1,1,1,1,1),
	packWord12(0,1,0,1,0,0,0,0,0,1,0,1),
	packWord12(0,0,0,0,1,1,0,1,1,0,0,0)
};

const uint32_t sprite_alien_middle_out_12x8[] =
{
	packWord12(0,0,0,1,0,0,0,0,0,1,0,0),
	packWord12(0,1,0,0,1,0,0,0,1,0,0,1),
	packWord12(0,1,0,1,1,1,1,1,1,1,0,1),
	packWord12(0,1,1,1,0,1,1,1,0,1,1,1),
	packWord12(0,1,1,1,1,1,1,1,1,1,1,1),
	packWord12(0,0,1,1,1,1,1,1,1,1,1,0),
	packWord12(0,0,0,1,0,0,0,0,0,1,0,0),
	packWord12(0,0,1,0,0,0,0,0,0,0,1,0)
};

const uint32_t sprite_alien_bottom_in_12x8[] =
{
	packWord12(0,0,0,0,1,1,1,1,0,0,0,0),
	packWord12(0,1,1,1,1,1,1,1,1,1,1,0),
	packWord12(1,1,1,1,1,1,1,1,1,1,1,1),
	packWord12(1,1,1,0,0,1,1,0,0,1,1,1),
	packWord12(1,1,1,1,1,1,1,1,1,1,1,1),
	packWord12(0,0,1,1,1,0,0,1,1,1,0,0),
	packWord12(0,1,1,0,0,1,1,0,0,1,1,0),
	packWord12(0,0,1,1,0,0,0,0,1,1,0,0)
};

const uint32_t sprite_alien_bottom_out_12x8[] =
{
	packWord12(0,0,0,0,1,1,1,1,0,0,0,0),
	packWord12(0,1,1,1,1,1,1,1,1,1,1,0),
	packWord12(1,1,1,1,1,1,1,1,1,1,1,1),
	packWord12(1,1,1,0,0,1,1,0,0,1,1,1),
	packWord12(1,1,1,1,1,1,1,1,1,1,1,1),
	packWord12(0,0,0,1,1,0,0,1,1,0,0,0),
	packWord12(0,0,1,1,0,1,1,0,1,1,0,0),
	packWord12(1,1,0,0,0,0,0,0,0,0,1,1)
};

const uint32_t sprite_tank_17x8[] =
{
	packWord17(0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0),
	packWord17(0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0),
	packWord17(0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0),
	packWord17(0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0),
	packWord17(0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0),
	packWord17(0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0),
	packWord17(0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0),
	packWord17(0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0)
};


// Shape of the entire bunker.
const uint32_t sprite_bunker_24x18[] =
{
	packWord24(0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0),
	packWord24(0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0),
	packWord24(0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0),
	packWord24(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1),
	packWord24(1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1)
};

// These are the blocks that comprise the bunker and each time a bullet
// strikes one of these blocks, you erode the block as you sequence through
// these patterns.
const uint32_t sprite_bunkerDamage0_6x6[] = {
	packWord6(0,1,1,0,0,0),
	packWord6(0,0,0,0,0,1),
	packWord6(1,1,0,1,0,0),
	packWord6(1,0,0,0,0,0),
	packWord6(0,0,1,1,0,0),
	packWord6(0,0,0,0,1,0)
};

const uint32_t sprite_bunkerDamage1_6x6[] = {
	packWord6(1,1,1,0,1,0),
	packWord6(1,0,1,0,0,1),
	packWord6(1,1,0,1,1,1),
	packWord6(1,0,0,0,0,0),
	packWord6(0,1,1,1,0,1),
	packWord6(0,1,1,0,1,0)
};

const uint32_t sprite_bunkerDamage2_6x6[] = {
	packWord6(1,1,1,1,1,1),
	packWord6(1,0,1,1,0,1),
	packWord6(1,1,0,1,1,1),
	packWord6(1,1,0,1,1,0),
	packWord6(0,1,1,1,0,1),
	packWord6(1,1,1,1,1,1)
};

const uint32_t sprite_bunkerDamage3_6x6[] = {
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1)
};

const uint32_t sprite_bunker_upper_left_gone_6x6[] = {
	packWord6(0,0,0,1,1,1),
	packWord6(0,0,1,1,1,1),
	packWord6(0,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1)
};

const uint32_t sprite_bunker_upper_right_gone_6x6[] = {
	packWord6(1,1,1,0,0,0),
	packWord6(1,1,1,1,0,0),
	packWord6(1,1,1,1,1,0),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1)
};

const uint32_t sprite_bunker_lower_left_gone_6x6[] = {
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(0,0,1,1,1,1),
	packWord6(0,0,0,1,1,1),
	packWord6(0,0,0,0,1,1),
	packWord6(0,0,0,0,0,1)
};

const uint32_t sprite_bunker_lower_right_gone_6x6[] = {
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,1,1),
	packWord6(1,1,1,1,0,0),
	packWord6(1,1,1,0,0,0),
	packWord6(1,1,0,0,0,0),
	packWord6(1,0,0,0,0,0)
};

const uint32_t sprite_tank_explosion1_15x8[] = {
	packWord15(0,0,0,0,1,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,1,0,0,0,0,1,0,0,1,0,0,0,0),
	packWord15(0,0,0,0,1,0,1,0,0,1,0,1,0,0,0),
	packWord15(0,0,1,0,0,0,0,0,0,0,0,0,1,0,0),
	packWord15(0,0,0,0,0,1,0,1,1,0,0,0,0,0,0),
	packWord15(1,0,0,1,1,1,1,1,1,1,1,0,0,0,0),
	packWord15(0,0,1,1,1,1,1,1,1,1,1,1,0,0,0),
	packWord15(1,1,1,1,1,1,1,1,1,1,1,1,1,0,0)
};

const uint32_t sprite_tank_explosion2_15x8[] = {
	packWord15(1,0,0,0,0,0,1,0,0,0,0,0,1,0,0),
	packWord15(0,0,0,1,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,1,0,0,0,0,1,0,0,0,0,0,1,0,0),
	packWord15(0,0,0,0,0,1,0,0,0,0,1,0,0,0,1),
	packWord15(1,0,0,0,1,1,0,0,1,0,0,0,0,0,0),
	packWord15(0,0,0,1,1,1,1,1,1,1,0,0,1,0,0),
	packWord15(0,0,1,1,1,1,1,1,1,1,1,0,0,0,0),
	packWord15(1,1,1,1,1,1,1,1,1,1,1,1,1,0,0)
};

const uint32_t sprite_tank_gone_15x8[] = {
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
	packWord15(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)
};

const uint32_t sprite_tankbullet_1x5[] = {
	packWord1(1),
	packWord1(1),
	packWord1(1),
	packWord1(1),
	packWord1(1)
};

const uint32_t sprite_tankbullet_gone_1x5[] = {
	packWord1(0),
	packWord1(0),
	packWord1(0),
	packWord1(0),
	packWord1(0)
};

const uint32_t sprite_alienbullet1_down_3x5[] = {
	packWord3(0,1,0),
	packWord3(0,1,0),
	packWord3(0,1,0),
	packWord3(1,1,1),
	packWord3(0,1,0)
};

const uint32_t sprite_alienbullet1_up_3x5[] = {
	packWord3(0,1,0),
	packWord3(1,1,1),
	packWord3(0,1,0),
	packWord3(0,1,0),
	packWord3(0,1,0)
};

const uint32_t sprite_alienbullet2_up_3x5[] = {
	packWord3(0,1,0),
	packWord3(1,0,0),
	packWord3(0,1,0),
	packWord3(0,0,1),
	packWord3(0,1,0)
};

const uint32_t sprite_alienbullet2_down_3x5[] = {
	packWord3(0,1,0),
	packWord3(0,0,1),
	packWord3(0,1,0),
	packWord3(1,0,0),
	packWord3(0,1,0)
};


const uint32_t sprite_alienbullet2_gone_3x5[] = {
	packWord3(0,0,0),
	packWord3(0,0,0),
	packWord3(0,0,0),
	packWord3(0,0,0),
	packWord3(0,0,0),
};

const uint32_t sprite_letterA_5x5[] = {
	packWord5(0,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1)
};

const uint32_t sprite_letterB_5x5[] = {
	packWord5(1,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,0)
};

const uint32_t sprite_letterC_5x5[] = {
	packWord5(0,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,0,0),
	packWord5(0,1,1,1,1)
};

const uint32_t sprite_letterD_5x5[] = {
	packWord5(1,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,0)
};

const uint32_t sprite_letterE_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,1,1,1,0),
	packWord5(1,0,0,0,0),
	packWord5(1,1,1,1,1)
};

const uint32_t sprite_letterF_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,1,1,1,0),
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,0,0)
};

const uint32_t sprite_letterG_5x5[] = {
	packWord5(0,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,1,1),
	packWord5(1,0,0,0,1),
	packWord5(0,1,1,1,0)
};

const uint32_t sprite_letterH_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1)
};

const uint32_t sprite_letterI_5x5[] = {
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0)
};

const uint32_t sprite_letterJ_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(1,1,1,0,0)
};

const uint32_t sprite_letterK_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,1,0),
	packWord5(1,1,1,0,0),
	packWord5(1,0,0,1,0),
	packWord5(1,0,0,0,1)
};

const uint32_t sprite_letterL_5x5[] = {
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,0,0),
	packWord5(1,1,1,1,1)
};

const uint32_t sprite_letterM_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,1,0,1,1),
	packWord5(1,0,1,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1)
};

const uint32_t sprite_letterN_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,1,0,0,1),
	packWord5(1,0,1,0,1),
	packWord5(1,0,0,1,1),
	packWord5(1,0,0,0,1)
};

const uint32_t sprite_letterO_5x5[] = {
	packWord5(0,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(0,1,1,1,0)
};

const uint32_t sprite_letterP_5x5[] = {
	packWord5(0,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,0,0,0,0)
};

const uint32_t sprite_letterQ_5x5[] = {
	packWord5(0,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,1,0),
	packWord5(0,1,1,0,1)
};

const uint32_t sprite_letterR_5x5[] = {
	packWord5(1,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1)
};

const uint32_t sprite_letterS_5x5[] = {
	packWord5(0,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(0,1,1,1,0),
	packWord5(0,0,0,0,1),
	packWord5(1,1,1,1,0)
};

const uint32_t sprite_letterT_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0)
};

const uint32_t sprite_letterU_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(0,1,1,1,0)
};

const uint32_t sprite_letterV_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(0,1,0,1,0),
	packWord5(0,0,1,0,0)
};

const uint32_t sprite_letterW_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,1,0,1),
	packWord5(1,0,1,0,1),
	packWord5(0,1,0,1,0)
};

const uint32_t sprite_letterX_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(0,1,0,1,0),
	packWord5(0,0,1,0,0),
	packWord5(0,1,0,1,0),
	packWord5(1,0,0,0,1)
};

const uint32_t sprite_letterY_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(0,1,0,1,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0)
};

const uint32_t sprite_letterZ_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,1,0),
	packWord5(0,0,1,0,0),
	packWord5(0,1,0,0,0),
	packWord5(1,1,1,1,1)
};

const uint32_t sprite_number0_5x5[] = {
	packWord5(0,1,1,1,0),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(0,1,1,1,0)
};

const uint32_t sprite_number1_5x5[] = {
	packWord5(1,1,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(0,0,1,0,0),
	packWord5(1,1,1,1,1)
};

const uint32_t sprite_number2_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,1,1,1,1)
};

const uint32_t sprite_number3_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,0,1),
	packWord5(1,1,1,1,1)
};

const uint32_t sprite_number4_5x5[] = {
	packWord5(1,0,0,0,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,0,1),
	packWord5(0,0,0,0,1)
};

const uint32_t sprite_number5_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,0,1),
	packWord5(1,1,1,1,1)
};

const uint32_t sprite_number6_5x5[] = {
	packWord5(0,1,1,1,1),
	packWord5(1,0,0,0,0),
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1)
};

const uint32_t sprite_number7_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,0,1),
	packWord5(0,0,0,0,1),
	packWord5(0,0,0,0,1),
	packWord5(0,0,0,0,1)
};

const uint32_t sprite_number8_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1)
};

const uint32_t sprite_number9_5x5[] = {
	packWord5(1,1,1,1,1),
	packWord5(1,0,0,0,1),
	packWord5(1,1,1,1,1),
	packWord5(0,0,0,0,1),
	packWord5(1,1,1,1,1)
};

const uint32_t sprite_letterBLANK_5x5[] = {
	packWord5(0,0,0,0,0),
	packWord5(0,0,0,0,0),
	packWord5(0,0,0,0,0),
	packWord5(0,0,0,0,0),
	packWord5(0,0,0,0,0)
};

// clang-format on