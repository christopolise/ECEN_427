#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("christopolise");
MODULE_DESCRIPTION("ECEn 427 Programmable Interval Timer Driver");

#define MODULE_NAME "pit_427"
#define CLASS_NAME "pit_427"
#define PIT_RUN_MASK 0x1
#define PIT_INT_EN_MASK 0x2
#define MINOR_NUM 0
#define DEVICE_CNT 1
#define SUCCESS 0
#define NO_FLAGS 0
#define SIZE_COMPENSATE 1
#define ENABLE_BIT 0x1
#define DISABLE_BIT 0x0
#define IRQ_BIT 0x1
#define NO_DATA 0
#define BYTE_OFFSET_FACTOR 4
#define BUF_LIMIT 1000
#define SUDO_PERMISH 0644
#define TIME_FACT 100
#define INIT_SPEED 10000
#define GAME_MODE 0x3
#define BASE_FACT 10
#define ATTR_ERROR -1
#define MAX_BUF_SIZE 80
#define FIRST_LETTER 0
#define NUM_OF_ATTR 4
#define PERIOD_IND 0
#define RUN_IND 1
#define INT_EN_IND 2

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Device Struct //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// This struct contains all variables for an individual PIT device. Although
// this driver will only support one device, it is good practice to structure
// device-specific variables this way.  That way if you were to extend your
// driver to multiple devices, you could simply have an array of these device
// structs.

struct pit_device {
  int minor_num;                // Device minor number
  struct cdev cdev;             // Character device structure
  struct platform_device *pdev; // Platform device pointer
  struct device *dev;           // device (/dev)
  phys_addr_t phys_addr;        // Physical address
  u32 mem_size;                 // Allocated mem space size
  u32 *virt_addr;               // Virtual address

  // Add any device-specific items to this that you need
  struct resource *mem_register; // Allocates a region of memory
};

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Driver Globals /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// This section is used to store any driver-level variables (ie. variables for
// the entire driver, not per device)

// Major number to use for devices managed by this driver
static int major_num;
static dev_t dev_num;
static struct class *cl;

// The PIT device - since this driver only supports one device, we don't
// need a list here, we can just use a single struct.
static struct pit_device pit;

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Forward function declarations //////////////////////////
////////////////////////////////////////////////////////////////////////////////
static int pit_init(void);
static void pit_exit(void);
static int pit_probe(struct platform_device *pdev);
static int pit_remove(struct platform_device *pdev);
static ssize_t pit_do_read(struct device *dev, struct device_attribute *attr,
                           char *buf);
static ssize_t pit_do_write(struct device *dev, struct device_attribute *attr,
                            const char *buf, size_t count);

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Driver Functions ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Sets up the device struct inside of the kernel
static struct of_device_id pit_of_match[] = {
    {
        .compatible = "byu,ecen427-pit",
    },
    {}};

MODULE_DEVICE_TABLE(of, pit_of_match);

// Matches the appropriate permissions and ownership of this module in the
// kernel
static struct platform_driver pit_platform_driver = {
    .probe = pit_probe,
    .remove = pit_remove,
    .driver =
        {
            .name = MODULE_NAME,
            .owner = THIS_MODULE,
            .of_match_table = pit_of_match,
        },
};

// Keeps track of all possible file operations to driver and matches them to
// appropriate function
struct file_operations pit_fops = {
    .owner = THIS_MODULE,
};

struct attribute *bundle[NUM_OF_ATTR];

static struct device_attribute period_dev_attr;
static struct device_attribute enable_timer_dev_attr;
static struct device_attribute enable_interrupts_dev_attr;
static struct attribute_group pit_attr_group;

// This section contains driver-level functions.  Remember, when you print
// logging information from these functions, you should use the pr_*() functions
// (ie. pr_info, pr_warning, pr_err, etc.)

// Register driver init/exit with Linux
module_init(pit_init);
module_exit(pit_exit);

// This is called when Linux loads your driver
// @param void - there is no param for this function
// RETURN: status of initialization
static int pit_init(void) {

  pr_info("%s: Initializing PIT Driver!\n", MODULE_NAME);
  int err;
  // Get a major number for the driver -- alloc_chrdev_region; // pg. 45, LDD3.
  err = alloc_chrdev_region(&dev_num, MINOR_NUM, DEVICE_CNT, MODULE_NAME);

  // Unable to allocate the device region
  if (err) {
    pr_err("Could not allocate char device region\n");
    goto alloc_chrdev_region_err;
  }
  major_num = MAJOR(dev_num);
  pit.minor_num = MINOR_NUM;
  pr_info("MAJOR NUM: %d\n", major_num);
  pr_info("MINOR NUM: 0\n");

  // Create a device class. -- class_create()
  cl = class_create(THIS_MODULE, CLASS_NAME);

  // Unable to create device class, throw error
  if (IS_ERR(cl)) {
    pr_err("Could not create class for this device\n");
    goto class_create_err;
  }
  pr_info("Created class: %s\n", CLASS_NAME);

  // Register the driver as a platform driver -- platform_driver_register
  err = platform_driver_register(&pit_platform_driver);

  // In case we could not register the driver
  if (err) {
    pr_err("Could not register platform driver\n");
    goto platform_driver_register_err;
  }
  pr_info("Platform driver has been registered with name: %s\n",
          pit_platform_driver.driver.name);

  // If any of the above functions fail, return an appropriate linux error
  // code, and make sure you reverse any function calls that were
  // successful.
  pr_info("Driver init successful\n");
  return SUCCESS; //(Success)

// ERROR block

// Unregistering the device driver, destroy class
platform_driver_register_err:
  pr_info("Destroying driver class: %s\n", CLASS_NAME);
  class_destroy(cl);

// Destroying the class
class_create_err:
  pr_info("Unregistering the device major number: %d\n", major_num);
  unregister_chrdev_region(dev_num, DEVICE_CNT);

// Deallocating the region
alloc_chrdev_region_err:
  pr_info("ERR CODE: %d\n", err);
  return err;
}

// This is called when Linux unloads your driver
// Routine to run before removing module
// @param void - no params for this function
static void pit_exit(void) {
  pr_info("%s: Removing PIT Driver!\n", MODULE_NAME);
  // platform_driver_unregister
  pr_info("Unregistering the platform driver: %s\n",
          pit_platform_driver.driver.name);
  platform_driver_unregister(&pit_platform_driver);
  // class_destroy
  pr_info("Destroying driver class: %s\n", CLASS_NAME);
  class_destroy(cl);
  // unregister_chrdev_region
  pr_info("Unregistering the device major number: %d\n", major_num);
  unregister_chrdev_region(dev_num, DEVICE_CNT);
  return;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Device Functions ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// This section contains device-level functions.  Remember, when you print
// logging information from these functions, you should use the dev_*()
// functions
// (ie. dev_info, dev_warning, dev_err, etc.)

// Called by kernel when a platform device is detected that matches the
// 'compatible' name of this driver.
// @param pdev - platform struct for the driver
// RETURN: 0 on success, 1 on fail
static int pit_probe(struct platform_device *pdev) {

  int err;

  // Initialize the character device structure (cdev_init)
  cdev_init(&pit.cdev, &pit_fops);
  pit.cdev.owner = THIS_MODULE;
  pit.cdev.ops = &pit_fops;

  // Register the character device with Linux (cdev_add)
  err = cdev_add(&pit.cdev, dev_num, DEVICE_CNT);

  // In the case an PIT device could not be added
  if (err) {
    dev_err(pit.dev, "Could not add pit device\n");
    goto cdev_add_err;
  }

  // Create a device file in /dev so that the character device can be accessed
  // from user space (device_create).
  pit.dev = device_create(cl, NULL, dev_num, NULL, "pit_427");

  // In the case the device file could not be created
  if (IS_ERR(pit.dev)) {
    dev_err(pit.dev, "Could not create device file\n");
    goto device_create_err;
  }

  // Get the physical device address from the device tree --
  // platform_get_resource
  struct resource *rsrc_mem =
      platform_get_resource(pdev, IORESOURCE_MEM, NO_FLAGS);

  // In the case there is no matching device in the tree
  if (IS_ERR(rsrc_mem)) {
    dev_err(pit.dev, "MEM: Could not pull required info from device tree\n");
    // goto platform_get_resource_err_mem;
  }
  pit.phys_addr = rsrc_mem->start;
  pit.mem_size = rsrc_mem->end - rsrc_mem->start + SIZE_COMPENSATE;
  dev_info(pit.dev, "Set physical addr: %x\n", pit.phys_addr);
  dev_info(pit.dev, "Memory size: %d\n", pit.mem_size);

  // Reserve the memory region -- request_mem_region
  // Get a (virtual memory) pointer to the device -- ioremap
  pit.mem_register = request_mem_region(pit.phys_addr, pit.mem_size, "pit_427");

  // In the case memory cannot be allocated for device
  if (IS_ERR(pit.mem_register)) {
    dev_err(pit.dev, "Could not allocate memory region for device\n");
    goto request_mem_region_err;
  }
  dev_info(pit.dev, "Reserved memory\n");

  pit.virt_addr = ioremap(pit.phys_addr, pit.mem_size);

  // In the case no virtual address could be created for device
  if (IS_ERR(pit.virt_addr)) {
    dev_err(pit.dev, "Could not create virtual address for device\n");
    goto ioremap_err;
  }
  dev_info(pit.dev, "Virt addr: %p\n", pit.virt_addr);

  // Assign appropriate name, mode, and funct pointer to period attr
  period_dev_attr.attr.name = "period";
  period_dev_attr.attr.mode = SUDO_PERMISH; // rw-r--r--
  period_dev_attr.show = pit_do_read;
  period_dev_attr.store = pit_do_write;

  // Assign appropriate name, mode, and funct pointer to run attr
  enable_timer_dev_attr.attr.name = "run";
  enable_timer_dev_attr.attr.mode = SUDO_PERMISH; // rw-r--r--
  enable_timer_dev_attr.show = pit_do_read;
  enable_timer_dev_attr.store = pit_do_write;

  // Assign appropriate name, mode, and funct pointer to int_en attr
  enable_interrupts_dev_attr.attr.name = "int_en";
  enable_interrupts_dev_attr.attr.mode = SUDO_PERMISH; // rw-r--r--
  enable_interrupts_dev_attr.show = pit_do_read;
  enable_interrupts_dev_attr.store = pit_do_write;

  // Handles all attrs as a group so that err recovery is easier
  bundle[PERIOD_IND] = &period_dev_attr.attr;
  bundle[RUN_IND] = &enable_timer_dev_attr.attr;
  bundle[INT_EN_IND] = &enable_interrupts_dev_attr.attr;
  bundle[NUM_OF_ATTR - 1] = NULL;

  pit_attr_group.attrs = bundle;

  err = sysfs_create_group(&pit.dev->kobj, &pit_attr_group);

  // In case that we could not register all attributes as a group successfully
  if (err) {
    dev_err(pit.dev,
            "SYSFS: Could not create device attribute group successfully\n");
    goto sysfs_create_group_err;
  }

  iowrite32(GAME_MODE, pit.virt_addr);                  // turn on control bits
  iowrite32(INIT_SPEED * TIME_FACT, pit.virt_addr + 1); // set period
  // If any of the above functions fail, return an appropriate linux error
  // code, and make sure you reverse any function calls that were
  // successful.

  dev_info(pit.dev, "Probe was successfull\n");

  return SUCCESS; //(success)

  // ERROR block

sysfs_create_group_err:
  dev_err(pit.dev,
          "SYSFS: Could not create device attribute group successfully\n");
  sysfs_remove_group(&pit.dev->kobj, &pit_attr_group);

ioremap_err:
  iounmap(pit.virt_addr);
request_mem_region_err:
  release_mem_region(pit.mem_register->start, pit.mem_size);
device_create_err:
  device_destroy(cl, dev_num);
cdev_add_err:
  cdev_del(&pit.cdev);
  return err;
}

// Called when the platform device is removed
// @param pdev - platform_device struct to represent the device
// RETURN: EXIT_SUCCESS or EXIT_FAIL on performance of directives
static int pit_remove(struct platform_device *pdev) {

  sysfs_remove_group(&pit.dev->kobj, &pit_attr_group);

  // iounmap
  iounmap(pit.virt_addr);
  // release_mem_region
  release_mem_region(pit.mem_register->start, pit.mem_size);
  // device_destroy
  device_destroy(cl, dev_num);
  // cdev_del
  cdev_del(&pit.cdev);
  return SUCCESS;
}

// Function is called when an attribute is asked to show something (or basically
// read its val)
// @param dev - Pointer to device so that we know which dev is calling the show
// @param attr - Pointer to actual attribute that will be in the sysfs that we
// want info from
// @param buf - string in which info is transported to userspace
// RETURN: size of bytes returned to userspace
static ssize_t pit_do_read(struct device *dev, struct device_attribute *attr,
                           char *buf) {
  char opt = attr->attr.name[FIRST_LETTER];
  u32 val;

  pr_info("SYSFS: Reading value from %s attr...\n", attr->attr.name);

  // Based on first letter of attr name, will read from correct register and
  // report value back to sysfs
  switch (opt) {
  case 'p':
    val = ioread32(pit.virt_addr + 1);
    val /= TIME_FACT;
    pr_info("Period val: %d\n", val);
    return scnprintf(buf, MAX_BUF_SIZE, "%s:%u\n", attr->attr.name, val);
  case 'r':
    val = ioread32(pit.virt_addr) & PIT_RUN_MASK;
    pr_info("Run val: %d\n", val);
    return scnprintf(buf, MAX_BUF_SIZE, "%s:%u\n", attr->attr.name, val);
  case 'i':
    val = (ioread32(pit.virt_addr) & PIT_INT_EN_MASK) ? 1 : 0;
    pr_info("Interrupt enable val: %d\n", val);
    return scnprintf(buf, MAX_BUF_SIZE, "%s:%u\n", attr->attr.name, val);
  default:
    pr_err("SYSFS: Could not read value from register\n");
    return ATTR_ERROR;
  }
}

// Function is called when an attribute is asked to store something (or
// basically write a val)
// @param dev - Pointer to device so that we know which dev is calling the store
// @param attr - Pointer to actual attribute that will be in the sysfs that we
// want to write info to
// @param buf - string in which info written to specified register based on attr
// RETURN: size of bytes returned to userspace
static ssize_t pit_do_write(struct device *dev, struct device_attribute *attr,
                            const char *buf, size_t count) {
  char opt = attr->attr.name[FIRST_LETTER];
  u32 num = simple_strtol(buf, NULL, BASE_FACT);

  u32 cur_ctrl = ioread32(pit.virt_addr);

  pr_info("SYSFS: Writing value to %s attr...\n", attr->attr.name);

  // Based on first letter of attr name, will write to correct register and
  // report size back to sysfs
  switch (opt) {
  case 'p':
    iowrite32(num * TIME_FACT, pit.virt_addr + 1);
    pr_info("Period val: %d written\n", num);
    return count;
  case 'r':
    iowrite32((PIT_INT_EN_MASK & cur_ctrl) + num, pit.virt_addr);
    pr_info("Run val: %d written\n", num);
    return count;
  case 'i':
    iowrite32((PIT_RUN_MASK & cur_ctrl) + (num << 1), pit.virt_addr);
    pr_info("Interrupt enable val: %d written\n", num);
    return count;
  default:
    pr_err("SYSFS: Could not write value to register\n");
    return ATTR_ERROR;
  }
}