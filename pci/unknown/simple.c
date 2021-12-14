#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/cdev.h>     /* char device */

#include <linux/pci.h>

#define VENDOR_ID 0x10EE
#define DEVICE_ID 0x7024
#define PCIE_BARS  4
#define MAX_DEVICES 2

static dev_t pci_dev_number;
static struct cdev * driver_object;
static struct class * pci_class;
static struct device * pci_prc;
static struct device * pci_irq_0;
static struct device * pci_irq_1;
static int msi_vec_num = 2; // Number of requested MSI interrupts
static int msi_0 = -1;
static int msi_1 = -1;
// Used for poll and select
static DECLARE_WAIT_QUEUE_HEAD(queue_vs0);
static DECLARE_WAIT_QUEUE_HEAD(queue_vs1);

static irqreturn_t pci_isr_0(int irq, void * dev_id) {
  printk(KERN_NOTICE "codec IRQ: interrupt handler 0. IRQ: %d\n", irq);
  wake_up_interruptible(&queue_vs0);
  return IRQ_HANDLED;
}

static irqreturn_t pci_isr_1(int irq, void * dev_id) {
  printk(KERN_NOTICE "codec IRQ: interrupt handler 1. IRQ: %d\n", irq);
  wake_up_interruptible(&queue_vs1);
  return IRQ_HANDLED;
}

static void* bars[PCIE_BARS] = {0};
static int device_init(struct pci_dev * pdev, const struct pci_device_id * id) {

  int i = 0;
  if (pci_enable_device(pdev))
    return -EIO;

  // Request memory regions for bar 0 to 2
  for (i = 0; i < PCIE_BARS; i++) {
    if (pci_request_region(pdev, i, "codec_pci") != 0) {
      dev_err( & pdev-> dev, "Bar %d - I/O address conflict for device \"%s\"\n", i, pdev-> dev.kobj.name);
      return -EIO;
    }
  }

  // DEBUG: Check if we are in memory space (which we should) or io space
  if ((pci_resource_flags(pdev, 0) & IORESOURCE_IO))
    printk(KERN_NOTICE "codec INIT: in io space\n");
  else if ((pci_resource_flags(pdev, 0) & IORESOURCE_MEM))
    printk(KERN_NOTICE "codec INIT: in mem_space\n");
 
  // This request enables MSI_enable in the hardware
  msi_vec_num = pci_alloc_irq_vectors(pdev, 1, msi_vec_num, PCI_IRQ_MSI);

  // msi_N will contain the IRQ number - see /proc/interrupts
  msi_0 = pci_irq_vector(pdev, 0);
  msi_1 = pci_irq_vector(pdev, 1);
  printk(KERN_NOTICE "codec INIT: nvec: %d\n", msi_vec_num);
  printk(KERN_NOTICE "codec INIT: msi_0: %d\n", msi_0);
  printk(KERN_NOTICE "codec INIT: msi_1: %d\n", msi_1);

  if (request_irq(msi_0, pci_isr_0, IRQF_SHARED, "codec_pci", pdev)) {
    dev_err(&pdev->dev, "codec INIT: IRQ MSI %d not free.\n", msi_0);
    goto cleanup;
  }

  if (request_irq(msi_1, pci_isr_1, IRQF_SHARED, "codec_pci", pdev)) {
    dev_err(&pdev->dev, "codec INIT: IRQ MSI %d not free.\n", msi_1);
    goto cleanup;
  }

  for (i = 0; i < PCIE_BARS; i++) {
    // Last parameter is the address space/length of each bar. Defined in the PCIe core.
    bars[i] = pci_iomap(pdev, i, pci_resource_len(pdev, i));
    if (bars[i] == NULL) {
      printk(KERN_ERR "codec INIT: bar %d allocation failed\n", i);
      goto cleanup;
    }

    printk(KERN_NOTICE "codec INIT: bar %d pointer: %p\n", i, bars[i]);
  }
  printk(KERN_NOTICE "codec INIT: loaded\n");
  return 0;

cleanup:
    for (i = 0; i < PCIE_BARS; i++) {
      if (bars[i] != NULL)
        pci_iounmap(pdev, bars[i]);
      pci_release_region(pdev, i);
    }
    
  return -EIO;
}

static void device_deinit(struct pci_dev * pdev) {
  int i = 0; 
  if (msi_0 >= 0)
    free_irq(msi_0, pdev);

  if (msi_1 >= 0)
    free_irq(msi_1, pdev);

  pci_free_irq_vectors(pdev);

  // release bar regions
  for (i = 0; i < PCIE_BARS; i++)
    pci_release_region(pdev, i);
  for (i = 0; i < PCIE_BARS; i++) {
    if (bars[i] != NULL)
      pci_iounmap(pdev, bars[i]);
  }

  pci_disable_device(pdev);
}

static ssize_t my_read(struct file *file_ptr, char __user *buf, size_t count, loff_t *position)
{
    return count;
}
// File operations not in this snipped
static struct file_operations fops = {
  .owner = THIS_MODULE,
  .read = my_read,
};

static struct pci_device_id pci_drv_tbl[] = {
  {
    VENDOR_ID,
    DEVICE_ID,
    PCI_ANY_ID,
    PCI_ANY_ID,
    0,
    0,
    0
  },
  {
    0,
  }
};

static struct pci_driver pci_drv = {
  .name = "codec_pci",
  .id_table = pci_drv_tbl,
  .probe = device_init,
  .remove = device_deinit
};

static int __init mod_init(void) {
  int i = 0;
  if (alloc_chrdev_region( & pci_dev_number, 0, MAX_DEVICES, "codec_pci") < 0)
    goto err;

  driver_object = cdev_alloc();
  if (driver_object == NULL)
    goto free_dev_number;

  driver_object->owner = THIS_MODULE;
  driver_object->ops = &fops;
  if (cdev_add(driver_object, pci_dev_number, MAX_DEVICES))
    goto free_cdev;

  pci_class = class_create(THIS_MODULE, "codec_pci");
  if (IS_ERR(pci_class)) {
    pr_err("codec MOD_INIT: no udev support available\n");
    goto free_cdev;
  }

  pci_prc = device_create(pci_class, NULL, MKDEV(MAJOR(pci_dev_number), MINOR(pci_dev_number) + 0), NULL, "%s", "codec_prc");
  pci_irq_0 = device_create(pci_class, NULL, MKDEV(MAJOR(pci_dev_number), MINOR(pci_dev_number) + 1), NULL, "codec_irq_%d", 0);
  pci_irq_1 = device_create(pci_class, NULL, MKDEV(MAJOR(pci_dev_number), MINOR(pci_dev_number) + 2), NULL, "codec_irq_%d", 1);
  if (pci_register_driver(&pci_drv) < 0) {
    for (i = 0; i < MAX_DEVICES; i++)
      device_destroy(pci_class, MKDEV(pci_dev_number, i));
    goto free_dev_number;
  }

  return 0;

free_cdev:
    kobject_put(&driver_object->kobj);
free_dev_number:
    unregister_chrdev_region(pci_dev_number, MAX_DEVICES);
err:
  printk("error occured\n");
  return -EIO;
}

static void __exit mod_exit(void) {
  int i = 0;
  pci_unregister_driver( & pci_drv);
  device_unregister(pci_prc);
  device_unregister(pci_irq_0);
  device_unregister(pci_irq_1);
  for (i = 0; i < MAX_DEVICES; i++) {
    device_destroy(pci_class, MKDEV(pci_dev_number, i));
  }
  class_destroy(pci_class);
  cdev_del(driver_object);
  unregister_chrdev_region(pci_dev_number, MAX_DEVICES);
}

MODULE_LICENSE("GPL");
module_init(mod_init);
module_exit(mod_exit);
