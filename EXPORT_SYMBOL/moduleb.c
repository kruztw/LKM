#include <linux/init.h>
#include <linux/module.h>

extern int module_a_init_func(void);
extern void module_a_exit_func(void);

static inline int __init module_b_init(void)
{
    return module_a_init_func();
}

static inline void __exit module_b_exit(void)
{
    module_a_exit_func();
}

MODULE_LICENSE("GPL");
module_init(module_b_init);
module_exit(module_b_exit);
