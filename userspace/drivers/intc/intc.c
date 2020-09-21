#include "intc.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define OPEN_ERROR -1
#define INTC_ERROR -1
#define INTC_MMAP_SIZE 0x0001000
#define INTC_MMAP_OFFSET 0x0
#define IER_OFFSET 0x0128
#define SIE_OFFSET 0x10
#define CIE_OFFSET 0x14
#define MER_OFFSET 0x1C
#define IAR_OFFSET 0x0C
#define ISR_OFFSET 0x00

static int fd;
static int addr;

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
    printf("uio example init error -- did you forget to sudo?\n");
    return INTC_ERROR;
  }

  // memory map the physical address of the hardware into virtual address space
  addr = mmap(NULL, INTC_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
              INTC_MMAP_OFFSET);
  if (addr == MAP_FAILED) {
    return INTC_ERROR;
  }

  /* put hardware setup here */
  generic_write(MER_OFFSET, 0x2);

  return INTC_SUCCESS;
}

// Called to exit the driver (unmap and close UIO file)
void intc_exit() {
  //
  munmap(addr, INTC_MMAP_SIZE);
  close(fd);
}

// This function will block until an interrupt occurrs
// Returns: Bitmask of activated interrupts
uint32_t intc_wait_for_interrupt() {
  //
  uint32_t fourbytes;
  read(fd, &fourbytes, sizeof(fourbytes));

  return generic_read(ISR_OFFSET);
}

// Acknowledge interrupt(s) in the interrupt controller
// irq_mask: Bitmask of interrupt lines to acknowledge.
void intc_ack_interrupt(uint32_t irq_mask) {
  //
  generic_write(IAR_OFFSET, irq_mask);
}

// // Instruct the UIO to enable interrupts for this device in Linux
// // (see the UIO documentation for how to do this)
void intc_enable_uio_interrupts() {
  //
  write(fd, 0x00000001, sizeof(char[4]));
}

// Enable interrupt line(s)
// irq_mask: Bitmask of lines to enable
// This function only enables interrupt lines, ie, a 0 bit in irq_mask
//	will not disable the interrupt line
void intc_irq_enable(uint32_t irq_mask) {
  //
  generic_write(SIE_OFFSET, irq_mask);
}

// Same as intc_irq_enable, except this disables interrupt lines
void intc_irq_disable(uint32_t irq_mask) {
  //
  generic_write(CIE_OFFSET, irq_mask);
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