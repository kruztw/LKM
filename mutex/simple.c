/* https://www.itread01.com/content/1550629628.html */
/* from simpliciy, remove some check */

#include <linux/types.h>
#include <linux/stat.h>
#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fcntl.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#define BUFF_SIZE 1024

dev_t my_dev;
struct class *my_class;

struct my_mutex{
    struct mutex mutex;
    char buff[0x100];
};

struct my_mutex mutex_buf;

static int open_file(struct inode *node,struct file *g_file)
{
    return 0;
}

static int close_file(struct inode *node,struct file *g_file)
{
    return 0;
}

static ssize_t read_file(struct file *g_file,char __user *buf,size_t size,loff_t *offt)
{
    int ret;
    size_t count = size;

    mutex_lock(&mutex_buf.mutex);
    copy_to_user(buf, mutex_buf.buff, count);
    *offt += count;
    ret = count;
    mutex_unlock(&mutex_buf.mutex);

    return ret;
}

static ssize_t write_file(struct file *g_file,const char __user *buf,size_t size,loff_t* offt)
{
    int ret;
    size_t count = size;

    mutex_lock(&mutex_buf.mutex);                 // 去掉這行會造成 race condition
    copy_from_user(mutex_buf.buff, buf, count);
    *offt += count;
    ret = count;
    mutex_unlock(&mutex_buf.mutex);               // 去掉這行

    return ret;
}

struct file_operations file_ops = {
    .owner = THIS_MODULE,
    .open = open_file,
    .release = close_file,
    .read = read_file,
    .write = write_file,
};

struct cdev dev = {
    .owner = THIS_MODULE,
};

static int my_init(void)
{
    int ret = 0;
    int i;
    
    ret = alloc_chrdev_region(&my_dev, 0, 1, "my_region");
    if(ret < 0)
        return ret;

    cdev_init(&dev, &file_ops);
    cdev_add(&dev, my_dev, 1);
    mutex_init(&mutex_buf.mutex);
    my_class = class_create(THIS_MODULE,"mutex_device");
    device_create(my_class, 0, MKDEV(MAJOR(my_dev),0) ,0,"my_dev");

    return 0;
}

static void my_exit(void)
{
    cdev_del(&dev);
    device_destroy(my_class, MKDEV(MAJOR(my_dev),0));
    class_destroy(my_class);
    unregister_chrdev_region(my_dev, 1);
}

MODULE_LICENSE("GPL v2");
module_init(my_init);
module_exit(my_exit);
