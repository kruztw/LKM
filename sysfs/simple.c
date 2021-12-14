//////////////////////////////////////////////////////////////////////////
// Usage:                                                               //
//        make                                                          //
//        insmod simple.ko                                              //
//        ls /sys/kernel/simple_module                                  //
//                                                                      //
// test :                                                               //
//        echo 1 > /sys/kernel/simple_module/attr1                      //
//        cat /sys/kernel/simple_module/attr1      # output: attr1: 1   //
//                                                                      //
// remove:                                                              //
//        rmmod simple                                                  //
//////////////////////////////////////////////////////////////////////////

#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>

static int attr1;
static int attr2;
static int attr3;

static ssize_t attr1_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "attr1: %d\n", attr1);
}

static ssize_t attr1_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    sscanf(buf, "%d", &attr1);
    return count;
}

static ssize_t attr23_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    if(!strcmp(attr->attr.name, "attr2"))
        return sprintf(buf, "attr2: %d\n", attr2);
    else
        return sprintf(buf, "attr3: %d\n", attr3);
}

static ssize_t attr23_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    if(!strcmp(attr->attr.name, "attr2"))
        sscanf(buf, "%d", &attr2);
    else
        sscanf(buf, "%d", &attr3);
    return count;
}

static struct kobj_attribute attr1_attribute = __ATTR(attr1, 0664, attr1_show , attr1_store);
static struct kobj_attribute attr2_attribute = __ATTR(attr2, 0664, attr23_show, attr23_store);
static struct kobj_attribute attr3_attribute = __ATTR(attr3, 0664, attr23_show, attr23_store);


static struct attribute *attrs[] = {
    &attr1_attribute.attr,
    &attr2_attribute.attr,
    &attr3_attribute.attr,
    NULL,
};


static struct attribute_group attr_group = {
    .attrs = attrs,
};

static struct kobject *kobj;

static int __init simple_init(void)
{
    int ret = -ENOMEM;

    kobj = kobject_create_and_add("simple_module", kernel_kobj);
    if(!kobj)
        return ret;

    ret = sysfs_create_group(kobj, &attr_group);
    if(ret)
      kobject_put(kobj);

    return ret;
}


static void __exit simple_exit(void)
{
    kobject_put(kobj);
}


module_init(simple_init);
module_exit(simple_exit);
MODULE_LICENSE("GPL");