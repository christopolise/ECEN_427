#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("christopolise");
MODULE_DESCRIPTION("ECEn 427 Audio Driver");

#define MODULE_NAME "audio"
#define CLASS_NAME "audio_class"

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
static ssize_t audio_read(struct file *filp, char __user *buff, size_t count, loff_t *offp);
static ssize_t audio_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);
// static int audio_ioctl(struct inode *, struct file *, unsigned int, unsigned long);
static irqreturn_t audio_isr(int irq, void * dev_id);

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

struct file_operations audio_fops =
    {
        .owner = THIS_MODULE,
        .llseek = NULL,
        .read = audio_read,
        .write = audio_write,
        // .unlocked_ioctl = audio_ioctl,
        .open = NULL,
        .release = NULL,
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
  pr_info("Platform driver has been registered with name: %s\n", audio_platform_driver.driver.name);

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
  pr_info("Unregistering the platform driver: %s\n", audio_platform_driver.driver.name);
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

  int err;
  int dev_no = MKDEV(major_num, 0);

  // Initialize the character device structure (cdev_init)
  cdev_init(&audio.cdev, &audio_fops);
  audio.cdev.owner = THIS_MODULE;
  audio.cdev.ops = &audio_fops;

  // Register the character device with Linux (cdev_add)
  err = cdev_add(&audio.cdev, dev_no, 1);
  if (err) {
    dev_err(audio.dev, "Could not add audio device\n");
    goto cdev_add_err;
  }

  // Create a device file in /dev so that the character device can be accessed
  // from user space (device_create).
  audio.dev = device_create(cl, NULL, dev_no, NULL, "ecen427_audio");
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
  dev_info(audio.dev, "Virt addr: %x\n", audio.virt_addr);

  // Get the IRQ number from the device tree -- platform_get_resource
  // Register your interrupt service routine -- request_irq
  struct resource *rsrc_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
  if (IS_ERR(rsrc_irq)) {
    dev_err(audio.dev, "IRQ: Could not pull required info from device tree\n");
    // goto platform_get_resource_err_irq;
  }

dev_info(audio.dev, "IRQ start val: %d\n", 0);
  err = request_irq(rsrc_irq->start, audio_isr, 0, "ecen427_audio", NULL);
  if(err)
  {
    dev_err(audio.dev, "IRQ: Could not set up manner to request IRQ\n");
    goto request_irq_err;
  }
  // iowrite32(__, audio.virt_addr);
  // If any of the above functions fail, return an appropriate linux error
  // code, and make sure you reverse any function calls that were
  // successful.

  return 0; //(success)


request_irq_err:
  dev_err(audio.dev, "IRQ: Could not set up manner to request IRQ\n");
  // freeirq()
// platform_get_resource_irq:

ioremap_err:
  iounmap(audio.phys_addr);
request_mem_region_err:
  release_mem_region(audio.phys_addr, audio.mem_size);
// platform_get_resource_err:
device_create_err:
  device_destroy(cl, dev_num);
cdev_add_err:
  cdev_del(&audio.cdev);
  return err;
}

// Called when the platform device is removed
static int audio_remove(struct platform_device *pdev) {

// freeirq

  // iounmap
  iounmap(audio.phys_addr);
  // release_mem_region
  release_mem_region(audio.phys_addr, audio.mem_size);
  // device_destroy
  device_destroy(cl, dev_num);
  // cdev_del
  cdev_del(&audio.cdev);
  return 0;
}

static ssize_t audio_read(struct file *filp, char __user *buff, size_t count, loff_t *offp) {
  pr_info("The read function was called\n");
  return 0;
}

static ssize_t audio_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp){
  pr_info("The write function was called\n");
  return 0;
}

static irqreturn_t audio_isr(int irq, void * dev_id)
{
  pr_info("The ISR function was called. ISR: %d\n", irq);
  return IRQ_HANDLED;
}
