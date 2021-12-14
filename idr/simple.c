#include <linux/module.h>
#include <linux/init.h>
#include <linux/idr.h>

struct idr map;

char *ptr1 = "apple";
char *ptr2 = "banana";

int deleter(int id, void *ptr, void *data) {
    idr_remove(&map, id);
    return 0;
}

int showKeyValuePair(int id, void *ptr, void *data) {
    printk("data = %s\n", (char *)data);
    printk("Key : %d | Value : %s\n", id, (char *)ptr);
    return 0;
}

static int __init mod_init(void) 
{
    int id[2];
    char *ptr;

    idr_init(&map);
    id[0] = idr_alloc(&map, (void *)ptr1, 0, 0, GFP_KERNEL);
    id[1] = idr_alloc(&map, (void *)ptr2, 0, 0, GFP_KERNEL);

    printk("Reserved ID's : {%d, %d}\n", id[0], id[1]);

    ptr = idr_find(&map, id[0]);
    printk("Find for ID %d = %s\n", id[0], (ptr != NULL) ? ptr : "not found");

    idr_for_each(&map, showKeyValuePair, "hello");

    return 0;
}

static void __exit mod_exit(void)
{
    idr_for_each(&map, deleter, NULL);
    idr_destroy(&map);
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
