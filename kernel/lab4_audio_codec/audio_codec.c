#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("christopolise");
MODULE_DESCRIPTION("ECEn 427 Audio Driver");

#define MODULE_NAME "audio"
#define CLASS_NAME "audio_class"
#define I2S_STATUS_REG_OFFSET 0x10 / 4
#define AUDIO_IOC_MAGIC 0xFE
#define AUDIO_IOC_ENABLE_LOOP _IO(AUDIO_IOC_MAGIC, 0)
#define AUDIO_IOC_DISABLE_LOOP _IO(AUDIO_IOC_MAGIC, 1)
#define AUDIO_IOC_MAXNR 3

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Device Struct //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// This struct contains all variables for an individual audio device. Although
// this driver will only support one device, it is good practice to structure
// device-specific variables this way.  That way if you were to extend your
// driver to multiple devices, you could simply have an array of these device
// structs.

struct audio_device {
  int minor_num;                // Device minor number
  struct cdev cdev;             // Character device structure
  struct platform_device *pdev; // Platform device pointer
  struct device *dev;           // device (/dev)
  phys_addr_t phys_addr;        // Physical address
  u32 mem_size;                 // Allocated mem space size
  u32 *virt_addr;               // Virtual address

  // Add any device-specific items to this that you need
  struct resource *mem_register; // Allocates a region of memory
  struct resource *rsrc_irq;     // IRQ interface

  char data_buffer[512000]; // ~512KB buffer for audio data
  char *ptr;
  u32 bytes_write;
  u32 buff_size;

  u8 loop_enable;
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

// The audio device - since this driver only supports one device, we don't
// need a list here, we can just use a single struct.
static struct audio_device audio;

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Forward function declarations //////////////////////////
////////////////////////////////////////////////////////////////////////////////
static int audio_init(void);
static void audio_exit(void);
static int audio_probe(struct platform_device *pdev);
static int audio_remove(struct platform_device *pdev);
static ssize_t audio_read(struct file *filp, char __user *buff, size_t count,
                          loff_t *offp);
static ssize_t audio_write(struct file *filp, const char __user *buff,
                           size_t count, loff_t *offp);
static long audio_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static irqreturn_t audio_isr(int irq, void *dev_id);

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Driver Functions ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static struct of_device_id audio_of_match[] = {
    {
        .compatible = "byu,ecen427-audio_codec",
    },
    {}};

MODULE_DEVICE_TABLE(of, audio_of_match);

static struct platform_driver audio_platform_driver = {
    .probe = audio_probe,
    .remove = audio_remove,
    .driver =
        {
            .name = MODULE_NAME,
            .owner = THIS_MODULE,
            .of_match_table = audio_of_match,
        },
};

struct file_operations audio_fops = {
    .owner = THIS_MODULE,
    .read = audio_read,
    .write = audio_write,
    .unlocked_ioctl = audio_ioctl,
};

// This section contains driver-level functions.  Remember, when you print
// logging information from these functions, you should use the pr_*() functions
// (ie. pr_info, pr_warning, pr_err, etc.)

// Register driver init/exit with Linux
module_init(audio_init);
module_exit(audio_exit);

// This is called when Linux loads your driver
static int audio_init(void) {

  pr_info("%s: Initializing Audio Driver!\n", MODULE_NAME);
  int err;
  // Get a major number for the driver -- alloc_chrdev_region; // pg. 45, LDD3.
  err = alloc_chrdev_region(&dev_num, 0, 1, MODULE_NAME);
  if (err) {
    pr_err("Could not allocate char device region\n");
    goto alloc_chrdev_region_err;
  }
  major_num = MAJOR(dev_num);
  audio.minor_num = 0;
  pr_info("MAJOR NUM: %d\n", major_num);
  pr_info("MINOR NUM: 0\n");

  // Create a device class. -- class_create()
  cl = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(cl)) {
    pr_err("Could not create class for this device\n");
    goto class_create_err;
  }
  pr_info("Created class: %s\n", CLASS_NAME);

  // Register the driver as a platform driver -- platform_driver_register
  err = platform_driver_register(&audio_platform_driver);
  if (err) {
    pr_err("Could not register platform driver\n");
    goto platform_driver_register_err;
  }
  pr_info("Platform driver has been registered with name: %s\n",
          audio_platform_driver.driver.name);

  // If any of the above functions fail, return an appropriate linux error
  // code, and make sure you reverse any function calls that were
  // successful.
  pr_info("Driver init successful\n");
  return 0; //(Success)

platform_driver_register_err:
  pr_info("Destroying driver class: %s\n", CLASS_NAME);
  class_destroy(cl);
class_create_err:
  pr_info("Unregistering the device major number: %d\n", major_num);
  unregister_chrdev_region(dev_num, 1);
alloc_chrdev_region_err:
  pr_info("ERR CODE: %d\n", err);
  return err;
}

// This is called when Linux unloads your driver
static void audio_exit(void) {
  pr_info("%s: Removing Audio Driver!\n", MODULE_NAME);
  // platform_driver_unregister
  pr_info("Unregistering the platform driver: %s\n",
          audio_platform_driver.driver.name);
  platform_driver_unregister(&audio_platform_driver);
  // class_destroy
  pr_info("Destroying driver class: %s\n", CLASS_NAME);
  class_destroy(cl);
  // unregister_chrdev_region
  pr_info("Unregistering the device major number: %d\n", major_num);
  unregister_chrdev_region(dev_num, 1);
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
static int audio_probe(struct platform_device *pdev) {

  audio.loop_enable = 0;

  int err;

  // Initialize the character device structure (cdev_init)
  cdev_init(&audio.cdev, &audio_fops);
  audio.cdev.owner = THIS_MODULE;
  audio.cdev.ops = &audio_fops;

  // Register the character device with Linux (cdev_add)
  err = cdev_add(&audio.cdev, dev_num, 1);
  if (err) {
    dev_err(audio.dev, "Could not add audio device\n");
    goto cdev_add_err;
  }

  // Create a device file in /dev so that the character device can be accessed
  // from user space (device_create).
  audio.dev = device_create(cl, NULL, dev_num, NULL, "ecen427_audio");
  if (IS_ERR(audio.dev)) {
    dev_err(audio.dev, "Could not create device file\n");
    goto device_create_err;
  }

  // Get the physical device address from the device tree --
  // platform_get_resource
  struct resource *rsrc_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  if (IS_ERR(rsrc_mem)) {
    dev_err(audio.dev, "MEM: Could not pull required info from device tree\n");
    // goto platform_get_resource_err_mem;
  }
  audio.phys_addr = rsrc_mem->start;
  audio.mem_size = rsrc_mem->end - rsrc_mem->start + 1;
  dev_info(audio.dev, "Set physical addr: %x\n", audio.phys_addr);
  dev_info(audio.dev, "Memory size: %d\n", audio.mem_size);

  // Reserve the memory region -- request_mem_region
  // Get a (virtual memory) pointer to the device -- ioremap
  audio.mem_register =
      request_mem_region(audio.phys_addr, audio.mem_size, "ecen427_audio");
  if (IS_ERR(audio.mem_register)) {
    dev_err(audio.dev, "Could not allocate memory region for device\n");
    goto request_mem_region_err;
  }
  dev_info(audio.dev, "Reserved memory\n");

  audio.virt_addr = ioremap(audio.phys_addr, audio.mem_size);
  if (IS_ERR(audio.virt_addr)) {
    dev_err(audio.dev, "Could not create virtual address for device\n");
    goto ioremap_err;
  }
  dev_info(audio.dev, "Virt addr: %p\n", audio.virt_addr);

  // Get the IRQ number from the device tree -- platform_get_resource
  // Register your interrupt service routine -- request_irq
  audio.rsrc_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
  if (IS_ERR(audio.rsrc_irq)) {
    dev_err(audio.dev, "IRQ: Could not pull required info from device tree\n");
    // goto platform_get_resource_err_irq;
  }

  dev_info(audio.dev, "IRQ start val: %d\n", audio.rsrc_irq->start);
  err =
      request_irq(audio.rsrc_irq->start, &audio_isr, 0, "ecen427_audio", NULL);
  if (err) {
    dev_err(audio.dev, "IRQ: Could not set up manner to request IRQ\n");
    goto request_irq_err;
  }
  dev_info(audio.dev, "Virt Addr: %p\nVirt Addr Offset: %p", audio.virt_addr,
           audio.virt_addr + I2S_STATUS_REG_OFFSET);
  iowrite32(0x1, audio.virt_addr + I2S_STATUS_REG_OFFSET);
  (ioread32(audio.virt_addr + I2S_STATUS_REG_OFFSET) & 0x1)
      ? pr_info("IRQs were enabled\n")
      : pr_info("IRQs have not been\n");
  // If any of the above functions fail, return an appropriate linux error
  // code, and make sure you reverse any function calls that were
  // successful.

  return 0; //(success)

request_irq_err:
  dev_err(audio.dev, "IRQ: Could not set up manner to request IRQ\n");
  free_irq(audio.rsrc_irq->start, NULL);
  // platform_get_resource_irq:

ioremap_err:
  iounmap(audio.virt_addr);
request_mem_region_err:
  release_mem_region(audio.mem_register->start, audio.mem_size);
// platform_get_resource_err:
device_create_err:
  device_destroy(cl, dev_num);
cdev_add_err:
  cdev_del(&audio.cdev);
  return err;
}

// Called when the platform device is removedd
static int audio_remove(struct platform_device *pdev) {

  free_irq(audio.rsrc_irq->start, NULL);

  // iounmap
  iounmap(audio.virt_addr);
  // release_mem_region
  release_mem_region(audio.mem_register->start, audio.mem_size);
  // device_destroy
  device_destroy(cl, dev_num);
  // cdev_del
  cdev_del(&audio.cdev);
  return 0;
}

static ssize_t audio_read(struct file *filp, char __user *buff, size_t count,
                   loff_t *offp) {
  pr_info("The read function was called\n");

  // return (ioread32((audio.virt_addr + I2S_STATUS_REG_OFFSET)) & 0x1FF800 &&
  //         ioread32(audio.virt_addr + I2S_STATUS_REG_OFFSET) & 0x1);

  u32 status = ioread32(audio.virt_addr + I2S_STATUS_REG_OFFSET);  // 0x10 / 4
  u32 bytes = (status >> 1) & 0x3FF; //0x3FF
  return (status & 1) && (bytes > 0);
}

static ssize_t audio_write(struct file *filp, const char __user *buff, size_t count,
                    loff_t *offp) {
  pr_info("The write function was called\n");

  iowrite32(0x0, audio.virt_addr + I2S_STATUS_REG_OFFSET);
  if (copy_from_user(audio.data_buffer, buff, count)) {
    return -EFAULT;
  }
  audio.ptr = audio.data_buffer;
  audio.bytes_write = count;
  audio.buff_size = count;
  iowrite32(0x1, audio.virt_addr + I2S_STATUS_REG_OFFSET);

  pr_info("Received val: %d\n", *(u32 *)audio.data_buffer);

  pr_info("Bytes read: %d\n", audio.bytes_write);

  return 0;
}

static irqreturn_t audio_isr(int irq, void *dev_id) {
  u32 left_remaining_bytes =
      (ioread32(audio.virt_addr + I2S_STATUS_REG_OFFSET) >> 11) & 0x3FF;

  if (audio.bytes_write < 4) {
    if (!left_remaining_bytes) {
      iowrite32(0x0, audio.virt_addr + I2S_STATUS_REG_OFFSET);
    }

    return IRQ_HANDLED;
  }

  left_remaining_bytes = 1000 - left_remaining_bytes;

  for (u16 i = 0; i < left_remaining_bytes; i++) {
    iowrite32(*(u32 *)audio.ptr, audio.virt_addr + 0xC / 4);
    iowrite32(*(u32 *)audio.ptr, audio.virt_addr + 0x8 / 4);
    audio.bytes_write -= 4;
    audio.ptr += 4;
    if (audio.bytes_write < 4) {
      if (audio.loop_enable) {
        audio.ptr = audio.data_buffer;
        audio.bytes_write = audio.buff_size;
        continue;
      }
      return IRQ_HANDLED;
    }
  }

  return IRQ_HANDLED;
}

static long audio_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {

  int err = 0;
  int retval = 0;

  if (_IOC_TYPE(cmd) != AUDIO_IOC_MAGIC)
    return -ENOTTY;
  if (_IOC_NR(cmd) > AUDIO_IOC_MAXNR)
    return -ENOTTY;

  if (_IOC_DIR(cmd) & _IOC_NONE)
    err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
  if (err)
    return -EFAULT;

  switch (cmd) {
  case AUDIO_IOC_ENABLE_LOOP:
    dev_info(audio.dev, "Audio looping enabled\n");
    audio.loop_enable = 1;
    return retval;
  case AUDIO_IOC_DISABLE_LOOP:
    dev_info(audio.dev, "Audio looping disabled\n");
    audio.loop_enable = 0;
    return retval;
  default:
    dev_err(audio.dev, "Invalid parameter passed!\n");
    return -ENOTTY;
  }
}