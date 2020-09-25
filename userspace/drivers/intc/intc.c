#include "intc.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define OPEN_ERROR -1
#define INTC_ERROR -1
#define INTC_MMAP_SIZE 0x0001000
#define INTC_MMAP_OFFSET 0x0
#define IER_OFFSET 0x08
#define SIE_OFFSET 0x10
#define CIE_OFFSET 0x14
#define MER_OFFSET 0x1C
#define IAR_OFFSET 0x0C
#define ISR_OFFSET 0x00
#define ENABLE_INTERRUPTS 0x7
#define ENABLE_MACHINE_INT 0x3

// Global variables that are the file descriptor for the device file opened for
// the INTC And an address descriptor for the location of the device
static int fd;
static char *addr;

// write to a register of the UIO device
void intc_generic_write(uint32_t offset, uint32_t value) {
  // the address is cast as a pointer so it can be dereferenced
  *((volatile uint32_t *)(addr + offset)) = value;
}

// read from a register of the UIO device
// Return: value read from the offset given
uint32_t intc_generic_read(uint32_t offset) {
  // Goes to offset and returns its value
  return *((volatile uint32_t *)(addr + offset));
}

// Initializes the driver (opens UIO file and calls mmap)
// devDevice: The file path to the uio dev file
// Returns: A negative error code on error, INTC_SUCCESS otherwise
// This must be called before calling any other intc_* functions
//
//  Tip: This function won't be able to open the UIO device file unless the
//  program is run with ''sudo''.  This is easy to forget, so it is helpful to
//  code an error message into this function that says "Did you forget to
//  sudo?", if it cannot open the UIO file.
int32_t intc_init(char devDevice[]) {
  fd = open(devDevice, O_RDWR);
  if (fd == OPEN_ERROR) {
    // Upon error returned by open, we exit function with similar error and
    // print out a msg to user
    printf("uio example init error -- did you forget to sudo?\n");
    return INTC_ERROR;
  }

  // memory map the physical address of the hardware into virtual address space
  addr = mmap(NULL, INTC_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
              INTC_MMAP_OFFSET);
  if (addr == MAP_FAILED) {
    // Upon error returned by mmap, we exit function with similar error
    return INTC_ERROR;
  }

  /* put hardware setup here */
  // Enables appropriate bits in IER and MER registers to allow the intc to know
  // which interrupts and what type will be needed for this program
  intc_generic_write(IER_OFFSET, ENABLE_INTERRUPTS);
  intc_generic_write(MER_OFFSET, ENABLE_MACHINE_INT);
  intc_enable_uio_interrupts();

  return INTC_SUCCESS;
}

// Called to exit the driver (unmap and close UIO file)
void intc_exit() {
  // Frees memory of virtual device file and closes the connection to the file
  munmap(addr, INTC_MMAP_SIZE);
  close(fd);
}

// This function will block until an interrupt occurrs
// Returns: Bitmask of activated interrupts
uint32_t intc_wait_for_interrupt() {
  // Create garbage buffer to write value received on device file, read is used
  // as a blocking function to wait for an interrupt
  uint32_t fourbytes;
  read(fd, &fourbytes, sizeof(fourbytes));
  return intc_generic_read(ISR_OFFSET);
}

// Acknowledge interrupt(s) in the interrupt controller
// irq_mask: Bitmask of interrupt lines to acknowledge.
void intc_ack_interrupt(uint32_t irq_mask) {
  // Write desired mask to IAR register to acknowledge receipt
  // of desired interrupts
  intc_generic_write(IAR_OFFSET, irq_mask);
}

// // Instruct the UIO to enable interrupts for this device in Linux
// // (see the UIO documentation for how to do this)
void intc_enable_uio_interrupts() {
  // Create a hex val of 1 to write to the device file to signal to the OS
  // that an interrupt has been received
  uint32_t enable = 0x00000001;
  write(fd, &enable, sizeof(enable));
}

// Enable interrupt line(s)
// irq_mask: Bitmask of lines to enable
// This function only enables interrupt lines, ie, a 0 bit in irq_mask
//	will not disable the interrupt line
void intc_irq_enable(uint32_t irq_mask) {
  // Write desired to the set IER register
  intc_generic_write(SIE_OFFSET, irq_mask);
}

// Same as intc_irq_enable, except this disables interrupt lines
void intc_irq_disable(uint32_t irq_mask) {
  // Write desired to the clear IER register
  intc_generic_write(CIE_OFFSET, irq_mask);
}
