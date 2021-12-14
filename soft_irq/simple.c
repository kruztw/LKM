#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
 
static void hello_printk(struct softirq_action *h)
{
    printk("hello world\n");
}
 
 
static int my_init(void)
{
    open_softirq(HI_SOFTIRQ, hello_printk);
    raise_softirq(HI_SOFTIRQ);
    return 0;
}
 
static void my_exit(void)
{
    raise_softirq_irqoff(HI_SOFTIRQ); // 中斷被關閉時仍然可以觸發 (不太懂)
}
 
module_init(my_init);
module_exit(my_exit);
