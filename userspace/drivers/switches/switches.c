#include "switches.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define OPEN_ERROR -1
#define SWITCHES_ERROR -1
#define SWITCHES_MMAP_SIZE 0x00010000
#define SWITCHES_MMAP_OFFSET 0x0
#define IP_IER_OFFSET 0x0128
#define GIER_OFFSET 0x011C
#define GPIO_DATA_OFFSET 0x0000
#define IP_ISR_OFFSET 0x0120
#define IP_IER_ENABLE 0x00000001
#define GIER_ENABLE 0x80000000
#define GIER_DISABLE 0x00000000
#define IP_ISR_PENDING_MASK 0x01
#define IP_ISR_ENABLE_INTERRUPT 0x00000001

// File descriptor used to associate all function activities with the buttons
// device
static int fd;
// Address assigned by mmap to allow reads and writes to registers
static char *addr;

// write to a register of the UIO device
void switches_generic_write(uint32_t offset, uint32_t value) {
  // the address is cast as a pointer so it can be dereferenced
  *((volatile uint32_t *)(addr + offset)) = value;
}

// read from a register of the UIO device
uint32_t switches_generic_read(uint32_t offset) {
  // Goes to offset and returns its value
  return *((volatile uint32_t *)(addr + offset));
}

// Initialize the driver
//  devFilePath: The file path to the uio dev file
//  Return: An error code on error, SWITCHES_SUCCESS otherwise
// This must be called before calling any other switches_* functions
//
//  Tip: This function won't be able to open the UIO device file unless the
//  program is run with ''sudo''.  This is easy to forget, so it is helpful to
//  code an error message into this function that says "Did you forget to
//  sudo?", if it cannot open the UIO file.
int32_t switches_init(char *devFilePath) {
  fd = open(devFilePath, O_RDWR);
  if (fd == OPEN_ERROR) {
    // Upon error returned by open, we exit function with similar error and
    // print out a msg to user
    printf("uio example init error -- did you forget to sudo?\n");
    return SWITCHES_ERROR;
  }

  // memory map the physical address of the hardware into virtual address space
  addr = mmap(NULL, SWITCHES_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
              SWITCHES_MMAP_OFFSET);
  if (addr == MAP_FAILED) {
    // Upon error returned by mmap, we exit function with similar error
    return SWITCHES_ERROR;
  }

  /* put hardware setup here */
  // Enabling interrupt in the IPIER
  switches_generic_write(IP_IER_OFFSET, IP_IER_ENABLE);
  switches_enable_interrupts();

  return SWITCHES_SUCCESS;
}

// Return the current state of the switches
uint8_t switches_read() {
  // Read value from GPIO and checks it against available switches, then returns
  // value
  return switches_generic_read(GPIO_DATA_OFFSET) & SWITCHES_ALL_MASK;
}

// Call this on exit to clean up
void switches_exit() {
  // Frees memory of virtual device file and closes the connection to the file
  munmap(addr, SWITCHES_MMAP_SIZE);
  close(fd);
}

// Enable GPIO interrupt output
void switches_enable_interrupts() {
  // Writes a 1 to appropriate bit in the general interrupt register to enable
  // it
  switches_generic_write(GIER_OFFSET, GIER_ENABLE);
}

// Disable GPIO interrupt output
void switches_disable_interrupts() {
  // Writes a 0 to appropriate bit in the general interrupt register
  switches_generic_write(GIER_OFFSET, GIER_DISABLE);
}

// Return whether an interrupt is pending
bool switches_interrupt_pending() {
  // Check for a one in bit 0 on IPISR to see if
  return switches_generic_read(IP_ISR_OFFSET) & IP_ISR_PENDING_MASK;
}

// Acknowledge a pending interrupt
void switches_ack_interrupt() {
  // Make sure the IPISR knows its okay to receive interrupts again
  switches_generic_write(IP_ISR_OFFSET, IP_ISR_ENABLE_INTERRUPT);
}
