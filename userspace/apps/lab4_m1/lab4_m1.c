#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

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
#define SUCCESS 0

// Runs audio driver test in userspace to see dmesg instances later
// returns 0 to indicate no errors in execution
int main() {

int fd; // File descriptor for the device file
int status; // Status value to be taken into consideration
char dummy_buf[DUMMY_BUF_LEN];  // Placeholder buffer needed to execute the read() and write() funcs


  printf(WELCOME_MSG);

  fd = open(AUDIO_DEVICE_FILE, O_RDWR);
  // If we cannot open the wave file
    if (fd == AUDIO_OPEN_ERROR) {
        // Upon error returned by open, we exit function with similar error and
        // print out a msg to user
        printf(ADMIN_CHK_MSG);
        printf(AUDIO_INIT_ERR_MSG);
        exit(AUDIO_OPEN_ERROR);
    }

  printf(READ_INTRO_MSG);    
  status = read(fd, dummy_buf, DUMMY_BUF_LEN);

  // If we cannot use the audio_read func
    if (status == READ_ERROR) {
        // If reading the buffer is invalid or cannot happen
        // Exits with a read error code and prints info to terminal
        printf(READ_ERR_MSG);
        exit(READ_ERROR);
    }


    printf(WRITE_INTRO_MSG);
    status = write(fd, dummy_buf, DUMMY_BUF_LEN);

    // If we cannot use the audio_write func
    if (status == WRITE_ERROR) {
        // If reading the buffer is invalid or cannot happen
        // Exits with a read error code and prints info to terminal
        printf(WRITE_ERR_MSG);
        exit(WRITE_ERROR);
    }

    printf(TEST_SUCC_MSG);
  return SUCCESS;
}