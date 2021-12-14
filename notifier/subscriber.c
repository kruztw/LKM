#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/notifier.h>

#include "notifier.h"

MODULE_LICENSE("GPL");

// callback function, 當my_notifier_list有事件發生時, 會呼叫該function
static int my_notify_sys(struct notifier_block *this, unsigned long val, void *data)
{
    printk("%s(): code=%ld, msg=\"%s\"\n", __FUNCTION__, val, (char*)data);
    return 0;
}

// 宣告要註冊到 my_notifier_list 的 struct
static struct notifier_block my_notifier =
{
        .notifier_call = my_notify_sys,
};

static int __init init_modules(void)
{
    // 將 my_notifier 註冊到 my_notifier_list
    register_my_notifier(&my_notifier);
    return 0;
}

static void __exit exit_modules(void)
{
    // 將my_notifier自my_notifier_list移除
    unregister_my_notifier(&my_notifier);
}

module_init(init_modules);
module_exit(exit_modules);
