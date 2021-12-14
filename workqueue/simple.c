#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

struct workqueue_struct *my_workqueue;

struct work_struct my_work;

void my_func(struct work_struct *work)
{
    printk("hello\n");
    /* do it again! rmmod simple will block, I don't know why. */
    //mdelay(10000); // busy waiting
    //msleep(10000); // context switch
    //queue_work(workqueue_test, &work_test);
}


static int my_init(void)
{
    my_workqueue = alloc_workqueue("cool_workqueue", 0, 0);
    INIT_WORK(&my_work, my_func);
    queue_work(my_workqueue, &my_work);
    return 0;
}

static void my_exit(void)
{
    destroy_workqueue(my_workqueue);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
