// https://zhuanlan.zhihu.com/p/43385422

#include <linux/module.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/delay.h>
 
 
static struct task_struct* old_task;
static struct wait_queue_head head;
 
int func_weakup(void* args)
{
   printk("func_wakeup...\n");
   printk("this thread pid = %d\n",current->pid);
   printk("init status, old_task->state = %ld\n",old_task->state);
 
   __wake_up(&head, TASK_NORMAL, 0, NULL); // 註解這行會導致 schedule_timeout_uninterruptible 等滿時間
 
   printk("after __wake_up, old_task->state = %ld\n",old_task->state);
   return 0;
}
 
static int my_init(void)
{
   char namefrt[] = "__wakeup.c:%s";
   long timeout;
   struct task_struct* result;
   struct wait_queue_entry data;

   result = kthread_create_on_node(func_weakup, NULL, -1, namefrt);
   printk("the new thread pid is : %d\n", result->pid);
   printk("the current pid is: %d\n", current->pid);
   init_waitqueue_head(&head);
   init_waitqueue_entry(&data,current);
   add_wait_queue(&head, &data);
   old_task = current;
   wake_up_process(result);
   timeout = schedule_timeout_uninterruptible(1000*3); // timeout 返回剩多少時間
   printk("sleep timeout = %ld\n",timeout);
   return 0;
}
 
static void my_exit(void)
{
}

MODULE_LICENSE("GPL");
module_init(my_init);
module_exit(my_exit);
