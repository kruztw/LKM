/*
 * make && make unload && make load && cat /dev/my_dev
 */

#include <linux/module.h>   /* THIS_MODULE */
#include <linux/cdev.h>     /* char device */
#include <linux/uaccess.h>  /* copy_to_user() */
#include <linux/fs.h> 	    /* file stuff */
#include <linux/kernel.h>   /* printk() */

char modname[] = "hello";
int my_major = 66; 

static ssize_t my_read(struct file *file_ptr, char __user *buf, size_t count, loff_t *position);
static struct file_operations my_fops =
{
    .owner = THIS_MODULE,
    .read = my_read,
};


static int __init my_init( void )
{
    return register_chrdev( my_major, modname, &my_fops);
}

static void __exit my_exit( void )
{
    unregister_chrdev( my_major, modname );
}

static const char hello[] = "Hello world!\n";
static ssize_t my_read(struct file *file_ptr, char __user *buf, size_t count, loff_t *position)
{
    if( *position + count > sizeof(hello) )
        count = sizeof(hello) - *position;
    else
        return 0;

    if( copy_to_user(buf, hello + *position, count) != 0 )
        return -EFAULT;

    *position += count;
    return count;
}

MODULE_LICENSE("GPL");
module_init( my_init );
module_exit( my_exit );

