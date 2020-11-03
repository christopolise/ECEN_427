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

// Runs audio driver test in userspace to see dmesg instances later
// returns 0 to indicate no errors in execution
int main(int argc, char * argv[]) {

  int fd; // File descriptor for the device file
  int audio_fd; // Audio file pointer

  int16_t raw_audio[512000];
  int32_t processed_audio[512000];

  if(!argc)
  {
    printf("Need an argument to play!\n");
    exit(-1);
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
  audio_fd = open(argv[1], O_RDONLY);
  if (audio_fd == AUDIO_OPEN_ERROR)
  {
    printf("Error opening wav file\n");
    exit(AUDIO_OPEN_ERROR);
  }
  lseek(audio_fd, 44, SEEK_SET);  // Just skipping headers
  printf("Opened WAV file successfully\n");

  // Read in raw data
  int bytes_read = read(audio_fd, raw_audio, 128000 * sizeof(int16_t));
  if(bytes_read == -1)
  {
    printf("ERROR reading the wav file\n");
    exit(-1);
  }
  printf("Bytes read: %d", bytes_read);

  for(uint16_t i = 0; i < bytes_read/2; i++)
  {
    processed_audio[i] = raw_audio[i];
    processed_audio[i] <<= 8;
  }

  int status_write;
  for (uint8_t i = 0; i < 2; i++)
  { 
    status_write = write(fd, processed_audio, bytes_read);
    if(status_write == -1)
    {
      printf("ERROR playing the wav file\n");
      exit(-1);
    }
    usleep(500000);
  }

  close(audio_fd);
  close(fd);

  return 0;
}