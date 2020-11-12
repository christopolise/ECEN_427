#include "sounds.h"
#include "globals.h"
#include <audio_config/audio_config.h>
#include <buttons/buttons.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <switches/switches.h>
#include <sys/ioctl.h>

enum soundVolumeControl {
  init_st,           // Machine is disabled
  wait_for_input_st, // Waiting for user input
  adjust_vol_st,     // Adjusts the volume
} currentSoundsHandler;

static int fd;
static bool enable_volume_control = false;
static uint32_t enablePress = 0;

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
#define INVALID_SIZE -1
#define HEADER_SIZE 44
#define SAMPLE_CONVERSION_FACTOR 2
#define SAMPLE_SHIFT_AMNT 8
#define GARBAGE_BUF_SIZE 2
#define SOUND_PLAYER_ERROR -1
#define MAX_ENABLE_WAIT 5
#define MAX_VOL 63
#define MIN_VOL 0
#define RESET_CNT 0

static int8_t vol_level = VOL_LEVEL_START;

// Array contains all the filepaths for the sound
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

// Array contains all processed audio snippets
int32_t *processed_audio_files[NUM_OF_SOUND_FX] = {
    sounds_invader_die, sounds_laser, sounds_player_die,
    sounds_ufo_die,     sounds_ufo,   sounds_walk1,
    sounds_walk2,       sounds_walk3, sounds_walk4};

// Initialize all vals to invalid, array contains all files' size
int bytes_recvd[NUM_OF_SOUND_FX] = {INVALID_SIZE, INVALID_SIZE, INVALID_SIZE,
                                    INVALID_SIZE, INVALID_SIZE, INVALID_SIZE,
                                    INVALID_SIZE, INVALID_SIZE, INVALID_SIZE};

// Converts the sounds from 16 to 24 bps sample rate for correct playing
// @param wavFile - file to be processed
// @param processed_audio - output processed buffer
// @param bytes_read - pointer to update the size of processed_audio in
// corresponding array
void process_sounds(char *wavFile, int32_t *processed_audio, int *bytes_read) {
  int raw_audio[SOUND_FX_MAX_SIZE];
  int audio_fd; // Audio file filp
  audio_fd = open(wavFile, O_RDWR);

  // If there is an issue opening the WAVE file
  if (audio_fd == AUDIO_OPEN_ERROR) {
    printf("Error opening wav file\n");
    exit(AUDIO_OPEN_ERROR);
  }
  lseek(audio_fd, HEADER_SIZE, SEEK_SET); // Just skipping headers

  *bytes_read = read(audio_fd, raw_audio, SOUND_FX_MAX_SIZE * sizeof(int16_t));

  // If the file was not processed properly
  if (*bytes_read == INVALID_SIZE) {
    printf("ERROR reading the wav file\n");
    exit(SOUND_PLAYER_ERROR);
  }

  // For all samples in the file, shift over to compensate for size increase
  for (uint16_t i = 0; i < *bytes_read / SAMPLE_CONVERSION_FACTOR; i++) {
    processed_audio[i] = raw_audio[i];
    processed_audio[i] <<= SAMPLE_SHIFT_AMNT;
  }

  close(audio_fd);
}

// Returns whether a sound is currently playing or not and whether it is
// available
bool sounds_is_available() {
  int garbage_size = GARBAGE_BUF_SIZE;
  char garbage_buf[GARBAGE_BUF_SIZE];

  // If there is data, its busy, else its free
  if (read(fd, garbage_buf, garbage_size)) {
    return false;
  } else {
    return true;
  }
}

// Initializes the sound driver for userspace
// @param devFile - string that contains device file path
// RETURN: Status of action
int8_t sounds_init(char *devFile) {
  fd = open(devFile, O_RDWR);

  // If there is a problem opening the device
  if (fd == SOUNDS_OPEN_ERROR) {
    // Upon error returned by open, we exit function with similar error and
    // print out a msg to user
    printf(SOUNDS_INIT_ERR_MSG);
    return SOUNDS_OPEN_ERROR;
  }

  audio_config_init();

  // Process all the sounds to be used for the game and puts them in temp memory
  for (uint8_t i = 0; i < NUM_OF_SOUND_FX; i++)
    process_sounds(audio_files[i], processed_audio_files[i], &bytes_recvd[i]);

  enable_volume_control = true;

  return SOUNDS_SUCCESS;
}

// Function that plays desired sound from wave array
// @param sound - integer indicating the number of file to be played
void sounds_play(uint8_t sound) {
  int status_write =
      write(fd, processed_audio_files[sound], bytes_recvd[sound]);

  // If the playing was unsuccessful, emergency exit
  if (status_write == INVALID_SIZE) {
    printf("ERROR playing the wav file\n");
    exit(SOUND_PLAYER_ERROR);
  }
}

// Toggle whether driver loops or unloops sample being played
// @param enable - indicates whether or not to loop
void sounds_toggle_looping(bool enable) {
  ioctl(fd, enable ? AUDIO_IOC_ENABLE_LOOP : AUDIO_IOC_DISABLE_LOOP);
  globals_setLooping(enable);
}

// Returns whether or not the driver is looping the sounds
bool sounds_is_looping() { return globals_isLooping(); }

// Succesfully closes the userspace instance of the driver file
// @param fd - file descriptor of device driver file
void sounds_exit(int fd) {
  // If there is an error exiting from the driver file, emergency exit
  if (close(fd) == SOUNDS_CLOSE_ERROR) {
    // If there is an issue closing the hdmi device file
    // Prints error message and exits with error code
    printf(FILE_ERR_MSG);
    exit(SOUNDS_CLOSE_ERROR);
  }
}

// Executed on every tick to manage sound functions
void sounds_tick() {

  uint32_t buttons = buttons_read();
  uint32_t switches = switches_read();

  // Mealy Actions
  switch (currentSoundsHandler) {
  case init_st:
    // if the state machine has been properly initialized
    if (enable_volume_control)
      currentSoundsHandler = wait_for_input_st;
    break;
  case wait_for_input_st:
    if (enablePress > MAX_ENABLE_WAIT) {
      // If the button to change volume is pressed
      if (buttons & BUTTONS_3_MASK) {
        currentSoundsHandler = adjust_vol_st;
      }
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
    enablePress++;
    break;
  case adjust_vol_st:

    // If the switch 0 is the the up or down position, incdec accordingly
    if (switches & SWITCHES_0_MASK && buttons & BUTTONS_3_MASK) {
      vol_level++;
      if (vol_level == MAX_VOL + 1) {
        vol_level = MAX_VOL;
      }
    } else {
      vol_level--;
      if (vol_level == MIN_VOL - 1) {
        vol_level = MIN_VOL;
      }
    }
    printf("Vol level: %i\n", vol_level);
    audio_config_set_volume(vol_level);
    enablePress = RESET_CNT;

    break;
  default:
    break;
  }
}