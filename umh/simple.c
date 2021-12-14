// https://stxinu.blogspot.com/2018/08/user-mode-helpers.html

#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/signal.h>
#include <linux/sched/signal.h>

static struct task_struct * slam_thread = NULL;

static int run_umh_app(void)
{
    char *argv[] = {"/bin/touch", "/home/kruztw/slam.txt", NULL};
    static char *envp[] = {
        "HOME=/",
        "TERM=linux",
        "PATH=/sbin:/bin:/usr/bin:/usr/sbin",
        NULL
    };

    return call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
}

static int slam_func(void *data)
{
    allow_signal(SIGKILL);
    mdelay(1000);

    while(!signal_pending(current)) {
        /* run user-mode helpers */
        run_umh_app();
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(msecs_to_jiffies(5000));
    }

    return 0;
}

static __init int kthread_signal_example_init(void)
{
    slam_thread = kthread_run(slam_func, NULL, "slam");
    return 0;
}

static __exit void kthread_signal_example_exit(void)
{
    if(!IS_ERR(slam_thread))
        send_sig(SIGKILL, slam_thread, 1);
}

module_init(kthread_signal_example_init);
module_exit(kthread_signal_example_exit);
