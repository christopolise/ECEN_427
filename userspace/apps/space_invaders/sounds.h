#ifndef __SOUNDS_H__
#define __SOUNDS_H__

#include <stdbool.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SOUND_FX_MAX_SIZE 128000
#define NUM_OF_SOUND_FX 9
#define SOUNDS_INVADER_DIE_INDX 0
#define SOUNDS_LASER_INDX 1
#define SOUNDS_PLAYER_DIE_INDX 2
#define SOUNDS_UFO_DIE_INDX 3
#define SOUNDS_UFO_INDX 4
#define SOUNDS_WALK1_INDX 5
#define SOUNDS_WALK2_INDX 6
#define SOUNDS_WALK3_INDX 7
#define SOUNDS_WALK4_INDX 8
#define SOUNDS_DEVICE_FILE "/dev/ecen427_audio"

int8_t sounds_init(char *devFile);

void sounds_play(uint8_t sound);

void sounds_toggle_looping(bool enable);

bool sounds_is_looping();

void sounds_exit(int fd);

void sounds_tick();

#endif