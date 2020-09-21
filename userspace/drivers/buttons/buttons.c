#include "buttons.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define OPEN_ERROR -1
#define BUTTONS_ERROR -1
#define MMAP_OFFSET 0x0
#define BUTTONS_MMAP_SIZE 0x00010000
#define IP_IER_OFFSET 0x0128
#define GIER_OFFSET 0x011C
#define GPIO_DATA_OFFSET 0x0000
#define IP_ISR_OFFSET 0x0120

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
  // Enabling interrupt in the IPIER
  generic_write(IP_IER_OFFSET, 0x00000001);
  buttons_enable_interrupts();

  return BUTTONS_SUCCESS;
}

// Return the current state of the buttons
uint8_t buttons_read() {
  //
  return generic_read(GPIO_DATA_OFFSET) & BUTTONS_ALL_MASK;
}

// Call this on exit to clean up
void buttons_exit() {
  munmap(addr, BUTTONS_MMAP_SIZE);
  close(fd);
}

// Enable GPIO interrupt output
void buttons_enable_interrupts() {
  //
  generic_write(GIER_OFFSET, 0x80000000);
}

// Disable GPIO interrupt output
void buttons_disable_interrupts() {
  //
  generic_write(GIER_OFFSET, 0x00000000);
}

// Return whether an interrupt is pending
bool buttons_interrupt_pending() {
  // Check for a one in bit 0 on IPISR to see if
  return generic_read(IP_ISR_OFFSET) & 0x01;
}

// Acknowledge a pending interrupt
void buttons_ack_interrupt() {
  // Make sure the IPISR knows its okay to receive interrupts again
  generic_write(IP_ISR_OFFSET, 0x00000001);
}

// write to a register of the UIO device
void generic_write(uint32_t offset, uint32_t value) {
  // the address is cast as a pointer so it can be dereferenced
  *((volatile uint32_t *)(addr + offset)) = value;
}

// read from a register of the UIO device
uint32_t generic_read(uint32_t offset) {
  return *((volatile uint32_t *)(addr + offset));
}