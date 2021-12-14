// https://www.cxyzjd.com/article/weixin_45574485/108132539

/** 此代码运行不起来，只做示范用，因为security_add_hooks 和LSM_HOOK_INIT中的security_hook_heads并不导出，make时会报如下错

ERROR: "security_hook_heads" [/root/zyc/test/lsm/test.ko] undefined!
ERROR: "security_add_hooks" [/root/zyc/test/lsm/test.ko] undefined!

解决办法是通过kallsyms_lookup_name("xxxx")将xxx符号导出，至于是否需要重新再定义一次LSM_HOOK_INIT，有待测试

此代码编译测试环境为：Linux ubuntu 5.3.0-51-generic  其security_add_hooks函数和hook用的my_rename函数原型都与本文前面贴的代码有细微差异，建议按照自身系统对应的源代码文件去调用
*/

#include <linux/security.h>
#include <linux/module.h>
#include <linux/lsm_hooks.h>

static int my_rename(const struct path *old_dir, struct dentry *old_dentry, const struct path *new_dir, struct dentry *new_dentry)
{
    printk("you are rename some file\n");
    return 0;
}

struct security_hook_list hooks[] =
{
    LSM_HOOK_INIT(path_rename, my_rename),
};

static int lsm_init(void)
{
    security_add_hooks(hooks, 1, "simple");
    printk("start");
    return 0;
}

static void lsm_exit(void)
{
    // lsm 可以调用security_delete_hooks进行hook卸载，这里就不写了
    printk("remove\n");
}

MODULE_LICENSE("GPL");
module_init(lsm_init);
module_exit(lsm_exit);

