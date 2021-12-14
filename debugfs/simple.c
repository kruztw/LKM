// https://github.com/chadversary/debugfs-tutorial/blob/master/example2/debugfs_example2.c
// /sys/kernel/debug/

#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static struct dentry *dir = 0;
static u32 sum = 0;

static int my_write_op(void *data, u64 value)
{
    sum += value;
	return 0;
}

static int my_read_op(void *data, u64 *value)
{
    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(add_fops, my_read_op, my_write_op, "%llu\n");

int init_module(void)
{
    struct dentry *junk;
    dir = debugfs_create_dir("calc", 0);
    if (!dir) {
        printk(KERN_ALERT "debugfs_example2: failed to create /sys/kernel/debug/example2\n");
        return -1;
    }

    junk = debugfs_create_file("add", 0222, dir, NULL, &add_fops);
    if (!junk) {
        printk(KERN_ALERT "debugfs_example2: failed to create /sys/kernel/debug/example2/add\n");
        return -1;
    }

    debugfs_create_u32("sum", 0444, dir, &sum);

    return 0;
}

void cleanup_module(void)
{
    debugfs_remove_recursive(dir);
}
