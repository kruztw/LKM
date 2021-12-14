#include <linux/init.h>
#include <linux/module.h>

int module_a_init_func(void)
{
    printk("this is module a init func\n");
    return 0;
}

void module_a_exit_func(void)
{
    printk("this is module a exit func\n");
}


static inline int __init module_a_init(void)
{
    printk("this is module a init\n");
    return module_a_init_func();
}

static inline void __exit module_a_exit(void)
{
    printk("this is module a exit\n");
    module_a_exit_func();
}


EXPORT_SYMBOL(module_a_init_func);
EXPORT_SYMBOL(module_a_exit_func);


MODULE_LICENSE("GPL");
module_init(module_a_init);
module_exit(module_a_exit);
