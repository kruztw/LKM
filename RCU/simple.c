/* code: https://www.mdeditor.tw/pl/pHh9/zh-tw */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/rcupdate.h>
#include <linux/delay.h>

struct foo {
    int a;
    struct rcu_head rcu;
    struct list_head list;
};

static struct foo *g_pfoo = NULL;

LIST_HEAD(g_rcu_list);

struct task_struct *rcu_reader_t;
struct task_struct *rcu_updater_t;
struct task_struct *rcu_reader_list_t;
struct task_struct *rcu_updater_list_t;

/* 指標的Reader操作 */
static int rcu_reader(void *data)
{
    struct foo *p = NULL;
    int cnt = 2;

    while (cnt--) {
        msleep(100);
        rcu_read_lock();
        p = rcu_dereference(g_pfoo);
        pr_info("%s: a = %d\n", __func__, p->a);
        rcu_read_unlock();
    }
    return 0;
}

/*  回收處理操作 */
static void rcu_reclaimer(struct rcu_head *rh)
{
    struct foo *p = container_of(rh, struct foo, rcu);
    pr_info("%s: a = %d\n", __func__, p->a);
    kfree(p);
}

/* 指標的Updater操作 */
static int rcu_updater(void *data)
{
    int value = 1;
    int cnt = 2;

    while (cnt--) {
        struct foo *old;
        struct foo *new = (struct foo *)kzalloc(sizeof(struct foo), GFP_KERNEL);

        msleep(200);
        old = g_pfoo;
        *new = *g_pfoo;
        new->a = value;
        rcu_assign_pointer(g_pfoo, new);

        pr_info("%s: a = %d\n", __func__, new->a);
        call_rcu(&old->rcu, rcu_reclaimer); // 非同步回收

        value++;
    }

    return 0;
}

/* 連結串列的Reader操作 */
static int rcu_reader_list(void *data)
{
    struct foo *p = NULL;
    int cnt = 2;

    while (cnt--) {
        msleep(300);
        rcu_read_lock();
        list_for_each_entry_rcu(p, &g_rcu_list, list) {
            pr_info("%s: a = %d\n",  __func__, p->a);
        }
        rcu_read_unlock();
    }

    return 0;
}

/* 連結串列的Updater操作 */
static int rcu_updater_list(void *data)
{
    int cnt = 2;
    int value = 1000;

    while (cnt--) {
        msleep(100);
        struct foo *p = list_first_or_null_rcu(&g_rcu_list, struct foo, list);
        struct foo *q = (struct foo *)kzalloc(sizeof(struct foo), GFP_KERNEL);

        *q = *p;
        q->a = value;

        list_replace_rcu(&p->list, &q->list);
        pr_info("%s: a = %d\n",  __func__, q->a);
        synchronize_rcu(); // 同步回收
        kfree(p);

        value++; 
    }

    return 0;
}

static int rcu_test_init(void)
{
    struct foo *p;

    rcu_reader_t = kthread_run(rcu_reader, NULL, "rcu_reader");
    rcu_updater_t = kthread_run(rcu_updater, NULL, "rcu_updater");
    rcu_reader_list_t = kthread_run(rcu_reader_list, NULL, "rcu_reader_list");
    rcu_updater_list_t = kthread_run(rcu_updater_list, NULL, "rcu_updater_list");

    g_pfoo = (struct foo *)kzalloc(sizeof(struct foo), GFP_KERNEL);
    p = (struct foo *)kzalloc(sizeof(struct foo), GFP_KERNEL);
    list_add_rcu(&p->list, &g_rcu_list);

    return 0;
}

static void rcu_test_exit(void)
{
    kfree(g_pfoo);
    kfree(list_first_or_null_rcu(&g_rcu_list, struct foo, list));

    kthread_stop(rcu_reader_t);
    kthread_stop(rcu_updater_t);
    kthread_stop(rcu_reader_list_t);
    kthread_stop(rcu_updater_list_t);
}

module_init(rcu_test_init);
module_exit(rcu_test_exit);

MODULE_LICENSE("GPL");
