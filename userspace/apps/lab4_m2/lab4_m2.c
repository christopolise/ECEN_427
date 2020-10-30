#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <audio_config/audio_config.h>

// Runs audio driver test in userspace to see dmesg instances later
// returns 0 to indicate no errors in execution
int main() {

int fd; // File descriptor for the device file
int status; // Status value to be taken into consideration
char dummy_buf[DUMMY_BUF_LEN];  // Placeholder buffer needed to execute the read() and write() funcs


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

  
  


  return 0;
}