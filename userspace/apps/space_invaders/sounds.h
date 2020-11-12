#ifndef __SOUNDS_H__
#define __SOUNDS_H__

#include <fcntl.h>
#include <stdbool.h>
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

// Returns whether a sound is currently playing or not and whether it is
// available
bool sounds_is_available();

// Initializes the sound driver for userspace
// @param devFile - string that contains device file path
// RETURN: Status of action
int8_t sounds_init(char *devFile);

// Function that plays desired sound from wave array
// @param sound - integer indicating the number of file to be played
void sounds_play(uint8_t sound);

// Toggle whether driver loops or unloops sample being played
// @param enable - indicates whether or not to loop
void sounds_toggle_looping(bool enable);

// Returns whether or not the driver is looping the sounds
bool sounds_is_looping();

// Succesfully closes the userspace instance of the driver file
// @param fd - file descriptor of device driver file
void sounds_exit(int fd);

// Executed on every tick to manage sound functions
void sounds_tick();

#endif