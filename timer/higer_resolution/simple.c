// https://blog.csdn.net/lhl_blog/article/details/106824066

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
 
static struct hrtimer hr_timer;
static unsigned long interval= 1000; /* unit: ms */
struct timespec64 uptime, uptimeLast;
 

enum hrtimer_restart my_hrtimer_callback( struct hrtimer *timer )
{
    ktime_get_ts64(&uptime); 
    printk(KERN_INFO"hrtimer:%9lu sec, %9lu ns\n", (unsigned long) uptime.tv_sec, uptime.tv_nsec); 
    uptimeLast = uptime;
    hrtimer_forward_now(timer, ms_to_ktime(interval));
    
    return HRTIMER_RESTART;
}
 
static int __init module_hrtimer_init( void )
{
    static ktime_t ktime;
 
    hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);            // CLOCK_MONOTONIC (相對時間) <=> CLOCK_REALTIME (絕對時間)
    ktime = ms_to_ktime(interval);                                         // ms_to_ktime (精度: ms), ns_to_ktime (精度: ns)
 
    hr_timer.function = my_hrtimer_callback;                               // 設定 callback
    hrtimer_start(&hr_timer, ktime, HRTIMER_MODE_REL);
 
    ktime_get_ts64(&uptime); 
    uptimeLast = uptime;
    return 0;
}

static void __exit module_hrtimer_exit( void )
{
    hrtimer_cancel(&hr_timer);
}


module_init(module_hrtimer_init);
module_exit(module_hrtimer_exit);

MODULE_LICENSE("GPL");
