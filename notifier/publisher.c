#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include "notifier.h"

MODULE_LICENSE("GPL");
// 宣告一個新的notifier list – my_notifier_list
BLOCKING_NOTIFIER_HEAD(my_notifier_list);

// 訂閱my_notifier_list事件的wrapper function
int register_my_notifier(struct notifier_block *nb)
{
    return blocking_notifier_chain_register(&my_notifier_list, nb);
}
EXPORT_SYMBOL(register_my_notifier);


// 取消訂閱my_notifier_list事件的wrapper function
int unregister_my_notifier(struct notifier_block *nb)
{
    return blocking_notifier_chain_unregister(&my_notifier_list, nb);
}
EXPORT_SYMBOL(unregister_my_notifier);


static ssize_t write_proc(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
    char *p = kzalloc(sizeof(char) * count, GFP_KERNEL);
    copy_from_user(p, buf, count);
    printk("%s(): msg=\"%s\"\n", __FUNCTION__, p);

    // 將事件published給my_notifier_list的subscriber
    blocking_notifier_call_chain(&my_notifier_list, my_event1, (void*)p);
    kfree(p);
    return count;
}

static const struct file_operations my_ops = {
    .owner = THIS_MODULE,
    .write = write_proc,
};

static int __init init_modules(void)
{
    proc_create("my_notifier", 0, NULL, &my_ops);
    return 0;
}

static void __exit exit_modules(void)
{
    remove_proc_entry("my_notifier", NULL);
}

module_init(init_modules);
module_exit(exit_modules);
