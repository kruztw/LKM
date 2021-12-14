/*
 * make && make unload && make load && cat /proc/my_proc
 */

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static int my_show(struct seq_file *fd, void *v) {
  seq_printf(fd, "Hello World!\n");
  return 0;
}

static int my_open(struct inode *inode, struct  file *file) {
  return single_open(file, my_show, NULL);
}

static const struct file_operations my_fops = {
  .owner = THIS_MODULE,
  .open = my_open,
  .read = seq_read,
  .release = single_release,
};

static int __init my_init(void) {
  proc_create("my_proc", 0, NULL, &my_fops);
  return 0;
}

static void __exit my_exit(void) {
  remove_proc_entry("my_proc", NULL);
}

MODULE_LICENSE("GPL");
module_init( my_init );
module_exit( my_exit );
