#include sounds.h
#include globals.h
#include <buttons/buttons.h>
#include <stdbool.h>

enum soundVolumeControl {
  init_st,           // Machine is disabled
  wait_for_input_st, // Waiting for user input
  adjust_vol_st,     // Adjusts the volume
} currentSoundsHandler;

static int fd;
static bool enable_volume_control = false;

#define AUDIO_OPEN_ERROR -1
#define SOUNDS_OPEN_ERROR -1
#define SOUNDS_INIT_ERR_MSG                                                    \
  "Could not open the audio driver device file, are you running as root?\n"
#define SOUNDS_CLOSE_ERROR -1
#define FILE_ERR_MSG "File close error encountered\n"
#define AUDIO_IOC_MAGIC 0xFE
#define AUDIO_IOC_ENABLE_LOOP _IO(AUDIO_IOC_MAGIC, 0)
#define AUDIO_IOC_DISABLE_LOOP _IO(AUDIO_IOC_MAGIC, 1)

char *audio_files[NUM_OF_SOUND_FX] = {
    "invader_die.wav", "laser.wav", "player_die.wav", "ufo_die.wav", "ufo.wav",
    "walk1.wav",       "walk2.wav", "walk3.wav",      "walk4.wav"};

int32_t *processed_audio_files[NUM_OF_SOUND_FX] = {
    sounds_invader_die, sounds_laser, sounds_player_die,
    sounds_ufo_die,     sounds_ufo,   sounds_walk1,
    sounds_walk2,       sounds_walk3, sounds_walk4};

int bytes_recvd[NUM_OF_SOUND_FX] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};

void process_sounds(char *devFile, int32_t *processed_audio, int *bytes_read) {
  int raw_audio[SOUND_FX_MAX_SIZE];
  int audio_fd; // Audio file filp
  if (audio_fd == AUDIO_OPEN_ERROR) {
    printf("Error opening wav file\n");
    exit(AUDIO_OPEN_ERROR);
  }
  lseek(audio_fd, 44, SEEK_SET); // Just skipping headers
  printf("Opened WAV file successfully\n");

  bytes_read = read(audio_fd, raw_audio, SOUND_FX_MAX_SIZE * sizeof(int16_t));
  if (bytes_read == -1) {
    printf("ERROR reading the wav file\n");
    exit(-1);
  }
  printf("Bytes read: %d", bytes_read);

  for (uint16_t i = 0; i < bytes_read / 2; i++) {
    processed_audio[i] = raw_audio[i];
    processed_audio[i] <<= 8;
  }

  close(audio_fd);
}

void sounds_init(char *devFile) {
  fd = open(devFile, O_RDWR);
  if (fd == SOUNDS_OPEN_ERROR) {
    // Upon error returned by open, we exit function with similar error and
    // print out a msg to user
    printf(SOUNDS_INIT_ERR_MSG);
    return SOUNDS_OPEN_ERROR;
  }

  audio_config_init();

  for (uint8_t i = 0; i < NUM_OF_SOUND_FX; i++)
    process_sounds(audio_files[i], processed_audio_files[i], &bytes_read[i]);

  enable_volume_control = true;

  return SOUNDS_SUCCESS;
}

void sounds_play(int32_t *sound, int bytes_read) {
  int status_write = write(fd, sound, bytes_read);
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
    break;
  default:
    break;
  }
}