#include "sounds.h"
#include "globals.h"
#include <buttons/buttons.h>
#include <switches/switches.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <audio_config/audio_config.h>

enum soundVolumeControl {
  init_st,           // Machine is disabled
  wait_for_input_st, // Waiting for user input
  adjust_vol_st,     // Adjusts the volume
} currentSoundsHandler;

static int fd;
static bool enable_volume_control = false;

static int32_t sounds_invader_die[SOUND_FX_MAX_SIZE];
static int32_t sounds_laser[SOUND_FX_MAX_SIZE];
static int32_t sounds_player_die[SOUND_FX_MAX_SIZE];
static int32_t sounds_ufo_die[SOUND_FX_MAX_SIZE];
static int32_t sounds_ufo[SOUND_FX_MAX_SIZE];
static int32_t sounds_walk1[SOUND_FX_MAX_SIZE];
static int32_t sounds_walk2[SOUND_FX_MAX_SIZE];
static int32_t sounds_walk3[SOUND_FX_MAX_SIZE];
static int32_t sounds_walk4[SOUND_FX_MAX_SIZE];

#define AUDIO_OPEN_ERROR -1
#define SOUNDS_OPEN_ERROR -1
#define SOUNDS_INIT_ERR_MSG                                                    \
  "Could not open the audio driver device file, are you running as root?\n"
#define SOUNDS_CLOSE_ERROR -1
#define FILE_ERR_MSG "File close error encountered\n"
#define AUDIO_IOC_MAGIC 0xFE
#define AUDIO_IOC_ENABLE_LOOP _IO(AUDIO_IOC_MAGIC, 0)
#define AUDIO_IOC_DISABLE_LOOP _IO(AUDIO_IOC_MAGIC, 1)
#define SOUNDS_SUCCESS 0
#define VOL_LEVEL_START 31

static uint8_t vol_level = VOL_LEVEL_START;

char *audio_files[NUM_OF_SOUND_FX] = {
    "/home/byu/ecen427/userspace/sounds/invader_die.wav",
    "/home/byu/ecen427/userspace/sounds/laser.wav", 
    "/home/byu/ecen427/userspace/sounds/player_die.wav", 
    "/home/byu/ecen427/userspace/sounds/ufo_die.wav", 
    "/home/byu/ecen427/userspace/sounds/ufo.wav",
    "/home/byu/ecen427/userspace/sounds/walk1.wav",       
    "/home/byu/ecen427/userspace/sounds/walk2.wav", 
    "/home/byu/ecen427/userspace/sounds/walk3.wav",      
    "/home/byu/ecen427/userspace/sounds/walk4.wav"};

int32_t *processed_audio_files[NUM_OF_SOUND_FX] = {
    sounds_invader_die, sounds_laser, sounds_player_die,
    sounds_ufo_die,     sounds_ufo,   sounds_walk1,
    sounds_walk2,       sounds_walk3, sounds_walk4};

int bytes_recvd[NUM_OF_SOUND_FX] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};

void process_sounds(char *wavFile, int32_t *processed_audio, int *bytes_read) {
  int raw_audio[SOUND_FX_MAX_SIZE];
  int audio_fd; // Audio file filp
  audio_fd = open(wavFile, O_RDWR);
  if (audio_fd == AUDIO_OPEN_ERROR) {
    printf("Error opening wav file\n");
    exit(AUDIO_OPEN_ERROR);
  }
  lseek(audio_fd, 44, SEEK_SET); // Just skipping headers
  printf("Opened WAV file successfully\n");

  *bytes_read = read(audio_fd, raw_audio, SOUND_FX_MAX_SIZE * sizeof(int16_t));
  if (*bytes_read == -1) {
    printf("ERROR reading the wav file\n");
    exit(-1);
  }
  printf("Bytes read: %d\n", *bytes_read);

  for (uint16_t i = 0; i < *bytes_read / 2; i++) {
    processed_audio[i] = raw_audio[i];
    processed_audio[i] <<= 8;
  }

  close(audio_fd);
}

int8_t sounds_init(char *devFile) {
  fd = open(devFile, O_RDWR);
  if (fd == SOUNDS_OPEN_ERROR) {
    // Upon error returned by open, we exit function with similar error and
    // print out a msg to user
    printf(SOUNDS_INIT_ERR_MSG);
    return SOUNDS_OPEN_ERROR;
  }

  audio_config_init();

  for (uint8_t i = 0; i < NUM_OF_SOUND_FX; i++)
    process_sounds(audio_files[i], processed_audio_files[i], &bytes_recvd[i]);

  enable_volume_control = true;

  return SOUNDS_SUCCESS;
}

void sounds_play(uint8_t sound) {
  printf("Sounds size: %d\n", bytes_recvd[sound]);
  int status_write = write(fd, processed_audio_files[sound], bytes_recvd[sound]);
  if (status_write == -1) {
    printf("ERROR playing the wav file\n");
    exit(-1);
  }
}

void sounds_toggle_looping(bool enable) {
  ioctl(fd, enable ? AUDIO_IOC_ENABLE_LOOP : AUDIO_IOC_DISABLE_LOOP);
  globals_setLooping(enable);
}

bool sounds_is_looping() { return globals_isLooping(); }

void sounds_exit(int fd) {
  if (close(fd) == SOUNDS_CLOSE_ERROR) {
    // If there is an issue closing the hdmi device file
    // Prints error message and exits with error code
    printf(FILE_ERR_MSG);
    exit(SOUNDS_CLOSE_ERROR);
  }
}

void sounds_tick() {

  uint32_t buttons = buttons_read();
  uint32_t switches = switches_read();

  // Mealy Actions
  switch (currentSoundsHandler) {
  case init_st:
    if (enable_volume_control)
      currentSoundsHandler = wait_for_input_st;
    break;
  case wait_for_input_st:
    if (buttons & BUTTONS_3_MASK) {
      currentSoundsHandler = adjust_vol_st;
    }
    break;
  case adjust_vol_st:
    currentSoundsHandler = wait_for_input_st;
    break;
  default:
    break;
  }

  // Moore Actions
  switch (currentSoundsHandler) {
  case init_st:
    break;
  case wait_for_input_st:
    break;
  case adjust_vol_st:
  
    (switches & SWITCHES_0_MASK) ? vol_level++ : vol_level--;
    audio_config_set_volume(vol_level);

    break;
  default:
    break;
  }
}