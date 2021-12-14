#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>


static long my_ioctl(struct file *file, unsigned int command, unsigned long arg)
{
    pr_info("Command num: %u\n", command);
    return 0;
}

static int my_open(struct inode *inode, struct file *filp)
{
    pr_info("Ok, open.\n");
    return 0;
}

static int my_close(struct inode *inode, struct file *filp)
{
    pr_info("Ok, close.\n");
    return 0;
}

static ssize_t my_read(struct file *filp, char __user *buff, size_t len, loff_t *ppos)
{
    pr_info("call read %ld bytes\n", len);
    return 0;
}

static ssize_t my_write(struct file *filp, const char __user *buff, size_t len, loff_t *ppos)
{
    pr_info("call write %ld bytes\n", len);
    return len;
}

static const struct file_operations my_fops = {
    .owner             = THIS_MODULE,
    .open              = my_open,
    .release           = my_close,
    .read              = my_read,
    .write             = my_write,
    .unlocked_ioctl    = my_ioctl,

};

static struct miscdevice my_miscdev = {
    .name           = "my_misc",
    .fops           = &my_fops,
};

static int __init my_init(void)
{
    int err;
    err = misc_register(&my_miscdev);
    if(err)
        printk("cannot register misc device\n");

    return err;
}

static void my_exit(void)
{
    return misc_deregister(&my_miscdev);
}


MODULE_LICENSE("GPL");
module_init(my_init);
module_exit(my_exit);
