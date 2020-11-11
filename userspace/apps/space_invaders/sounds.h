#ifndef __SOUNDS_H__
#define __SOUNDS_H__

#include sounds.h
#include <audio_config/audio_config.h>

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

extern int32_t sounds_invader_die[SOUND_FX_MAX_SIZE];
extern int32_t sounds_laser[SOUND_FX_MAX_SIZE];
extern int32_t sounds_player_die[SOUND_FX_MAX_SIZE];
extern int32_t sounds_ufo_die[SOUND_FX_MAX_SIZE];
extern int32_t sounds_ufo[SOUND_FX_MAX_SIZE];
extern int32_t sounds_walk1[SOUND_FX_MAX_SIZE];
extern int32_t sounds_walk2[SOUND_FX_MAX_SIZE];
extern int32_t sounds_walk3[SOUND_FX_MAX_SIZE];
extern int32_t sounds_walk4[SOUND_FX_MAX_SIZE];

void sounds_init(char *devFile);

void sounds_play(int32_t *sound);

void toggle_looping(bool enable);

bool is_looping();

void sounds_exit(int fd);

#endif