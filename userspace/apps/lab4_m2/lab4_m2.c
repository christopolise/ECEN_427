#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <audio_config/audio_config.h>

#define AUDIO_OPEN_ERROR -1
#define AUDIO_INIT_ERR_MSG "Could not properly open the audio device :P"
#define READ_ERROR -1
#define READ_ERR_MSG "Could not read properly from audio device"
#define WRITE_ERROR -1
#define WRITE_ERR_MSG "Could not write properly from audio device"
#define READ_INTRO_MSG "Commencing read() function for driver: check dmesg for output\n"
#define WRITE_INTRO_MSG "Commencing write() function for driver: check dmesg for output\n"
#define TEST_SUCC_MSG "Writing and reading from audio device file successful. Please check dmesg for output\nDon't forget to remove audio_codec.ko\n"
#define WELCOME_MSG "This is the Lab 4 Milestone Userspace test for the ecen427_audio driver\nPlease make sure audio_codec.ko is inserted\n\n"
#define AUDIO_DEVICE_FILE "/dev/ecen427_audio"
#define ADMIN_CHK_MSG "uio example init error -- did you forget to sudo?\n"
#define DUMMY_BUF_LEN 10
#define MAX_BUF_SIZE 128000
#define SUCCESS 0
#define FAILURE -1
#define INVALID_FILE_ACTION -1
#define DELAY_MICROSECONDS 500000
#define WAVE_HEADER_SIZE 44
#define CONVERSION_SHIFT 8
#define SAMPLE_CONVERT_FACTOR 2
#define NUM_OF_PLAYS 2
#define AUDIO_PATH 1

// Runs audio driver test in userspace to see dmesg instances later
// returns 0 to indicate no errors in execution
int main(int argc, char * argv[]) {

  int fd; // File descriptor for the device file
  int audio_fd; // Audio file pointer

  int16_t raw_audio[MAX_BUF_SIZE];
  int32_t processed_audio[MAX_BUF_SIZE];

  if(!argc)
  {
    printf("Need an argument to play!\n");
    exit(FAILURE);
  }

  printf(WELCOME_MSG);

  fd = open(AUDIO_DEVICE_FILE, O_RDWR);
  if (fd == AUDIO_OPEN_ERROR) {
      // Upon error returned by open, we exit function with similar error and
      // print out a msg to user
      printf(ADMIN_CHK_MSG);
      printf(AUDIO_INIT_ERR_MSG);
      exit(AUDIO_OPEN_ERROR);
  }

  audio_config_init();

  // Open file and jump past all the headers :D
  audio_fd = open(argv[AUDIO_PATH], O_RDONLY);

  // If we cannot open the wave file
  if (audio_fd == AUDIO_OPEN_ERROR)
  {
    printf("Error opening wav file\n");
    exit(AUDIO_OPEN_ERROR);
  }
  lseek(audio_fd, WAVE_HEADER_SIZE, SEEK_SET);  // Just skipping headers
  printf("Opened WAV file successfully\n");

  // Read in raw data
  int bytes_read = read(audio_fd, raw_audio, MAX_BUF_SIZE * sizeof(int16_t));

  // If we could not read from the wave file
  if(bytes_read == INVALID_FILE_ACTION)
  {
    printf("ERROR reading the wav file\n");
    exit(FAILURE);
  }
  printf("Bytes read: %d", bytes_read);

  // For each sample, convert to 24 bps sample rate
  for(uint16_t i = 0; i < bytes_read/SAMPLE_CONVERT_FACTOR; i++)
  {
    processed_audio[i] = raw_audio[i];
    processed_audio[i] <<= CONVERSION_SHIFT;
  }

  int status_write;
  for (uint8_t i = 0; i < NUM_OF_PLAYS; i++)
  { 
    status_write = write(fd, processed_audio, bytes_read);

    // If we cannot write to the audio driver
    if(status_write == INVALID_FILE_ACTION)
    {
      printf("ERROR playing the wav file\n");
      exit(FAILURE);
    }
    usleep(DELAY_MICROSECONDS);
  }

  close(audio_fd);
  close(fd);

  return SUCCESS;
}
