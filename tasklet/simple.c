#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/interrupt.h> 

MODULE_LICENSE("GPL");

static struct tasklet_struct my_tasklet;  
static void my_handler (unsigned long data)
{
        printk(KERN_ALERT "tasklet_handler is running.\n");
} 

static int my_init(void)
{
	tasklet_init(&my_tasklet, my_handler, 0);
    tasklet_schedule(&my_tasklet);
	return 0;
}

static void my_exit(void)
{
	tasklet_kill(&my_tasklet);
}

module_init(my_init);
module_exit(my_exit);

