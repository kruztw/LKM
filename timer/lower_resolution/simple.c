// https://stackoverflow.com/questions/55333646/how-to-fix-error-implicit-declaration-of-function-setup-timer

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
 
static struct timer_list timer;

 
static void timer_cb(struct timer_list *unused)
{
    printk("call timer_cb\n");
    // step3: 重新激活定时器
    mod_timer(&timer, jiffies + msecs_to_jiffies(1000)); 
}
 
static int __init module_lrtimer_init( void )
{
    printk("low resolution timer init.\n");

    // step1: 定义并初始化定时器*
    timer_setup(&timer, timer_cb, 0);

    // step2: 修改定时器超时时间，并激活定时器
    mod_timer(&timer, jiffies + msecs_to_jiffies(1000/*ms*/));
    return 0;
}
 
 
static void __exit module_lrtimer_exit( void )
{
    printk("low resolution timer exit.\n");
    del_timer(&timer);
}
 
module_init(module_lrtimer_init);
module_exit(module_lrtimer_exit);

MODULE_LICENSE("GPL");
