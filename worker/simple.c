/* https://blog.csdn.net/eZiMu/article/details/79090698 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>

struct kthread_worker my_worker;
struct kthread_work   my_work;
struct task_struct *w = NULL;


void my_func(struct kthread_work *work)
{
    printk("hello\n");
    //msleep(3000); // rmmod 時會壞掉, 不知道為什麼
    //kthread_queue_work(&my_worker, &my_work);
}


static int my_init(void)
{
    kthread_init_worker(&my_worker);
    kthread_init_work(&my_work, my_func);
    w = kthread_run(kthread_worker_fn, &my_worker, "cool worker"); /* 參考 linux 的 encx24j600.c */
    kthread_queue_work(&my_worker, &my_work);
    return 0;
}

static void my_exit(void)
{
    kthread_destroy_worker(&my_worker);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
