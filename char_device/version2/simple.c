#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define VULN_MINORS (1)

static dev_t my_chr_devt;
static struct cdev my_cdev;
static struct class *my_class;
static struct device *my_device;


static int my_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t my_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{
    return 0;
}

static ssize_t my_write(struct file *file, const char *buffer, size_t length, loff_t *offset)
{
    return 0;
}

static long my_ioctl(struct file *file, unsigned int command, unsigned long arg)
{
    return 0;
}

static const struct file_operations my_fops = {
    .owner            = THIS_MODULE,
    .read            = my_read,
    .write             = my_write,
    .open             = my_open,
    .unlocked_ioctl    = my_ioctl,
    .release        = my_release,
};

static int __init my_init(void)
{
    int result = -ENOMEM;
    result = alloc_chrdev_region(&my_chr_devt, 0, 1, "my_region");
    if (result < 0){
        printk("alloc_chrdev_region failed.\n");
        goto unregister_chrdev;
    }

    cdev_init(&my_cdev, &my_fops);

    result = cdev_add(&my_cdev, my_chr_devt, 1);
    if (result){
        printk("cdev_add failed.\n");
        goto unregister_chrdev;
    }

    my_class = class_create(THIS_MODULE, "my_class");
    if(IS_ERR(my_class)){
        printk("class_create failed.\n");
        result = PTR_ERR(my_class);
        goto unregister_chrdev;
    }

    my_device = device_create(my_class, NULL, my_chr_devt, NULL, "my_dev");
    if(IS_ERR(my_device)){
        printk("device_create failed.\n");
        result = PTR_ERR(my_device);
        goto destroy_class;
    }
    return 0;


    destroy_class:
        class_destroy(my_class);
    unregister_chrdev:
        unregister_chrdev_region(my_chr_devt, VULN_MINORS);
    
    return result;
}

static void my_exit(void)
{
    device_destroy(my_class, my_chr_devt);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(my_chr_devt, VULN_MINORS);
}


MODULE_LICENSE("GPL");
module_init(my_init);
module_exit(my_exit);
