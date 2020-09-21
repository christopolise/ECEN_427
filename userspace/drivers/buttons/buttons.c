#include "buttons.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define OPEN_ERROR -1
#define BUTTONS_ERROR -1
#define MMAP_OFFSET 0x0
#define BUTTONS_MMAP_SIZE 0x00010000

static int fd;
static int addr;

int32_t buttons_init(char *devFilePath) {
  fd = open(devFilePath, O_RDWR);
  if (fd == OPEN_ERROR) {
    printf("uio example init error -- did you forget to sudo?\n");
    return BUTTONS_ERROR;
  }

  // memory map the physical address of the hardware into virtual address space
  addr = mmap(NULL, BUTTONS_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
              MMAP_OFFSET);
  if (addr == MAP_FAILED) {
    return BUTTONS_ERROR;
  }

  /* put hardware setup here */

  return BUTTONS_SUCCESS;
}

// Return the current state of the buttons
uint8_t buttons_read() { return *((volatile uint32_t *)(addr + MMAP_OFFSET)); }

// Call this on exit to clean up
void buttons_exit() {
  munmap(addr, BUTTONS_MMAP_SIZE);
  close(fd);
}

// Enable GPIO interrupt output
void buttons_enable_interrupts() {}

// Disable GPIO interrupt output
void buttons_disable_interrupts() {}

// Return whether an interrupt is pending
bool buttons_interrupt_pending() {}

// Acknowledge a pending interrupt
void buttons_ack_interrupt() {}