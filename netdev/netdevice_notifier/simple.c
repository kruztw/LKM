// code: https://stackoverflow.com/questions/62273254/how-to-write-linux-kernel-module-that-is-a-new-instance-in-each-network-namespac

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>

//處理網路裝置的啟動與禁用等事件
int my_netdev_event(struct notifier_block *this, unsigned long event, void *ptr)
{
   struct net_device *dev = *(struct net_device **)ptr;

    switch(event)
    {
        case NETDEV_UP:
             if(dev && dev->name)
                 printk("dev[%s] is up\n",dev->name);
             break;
        case NETDEV_DOWN:
             if(dev && dev->name)
                 printk("dev[%s] is down\n",dev->name);
                break;
        default:
             break;
    }

    return NOTIFY_DONE;
}          

//處理ip地址的改變事件
int my_inetaddr_event(struct notifier_block *this, unsigned long event, void *ptr)
{
    struct in_ifaddr *ifa = (struct in_ifaddr *)ptr;
    struct net_device *dev = NULL;

    if(ifa && ifa->ifa_dev)
        dev = ifa->ifa_dev->dev;

    switch(event)
    {
        case NETDEV_UP:
            if(dev && dev->name)
                 printk("inet[%s] is up\n",dev->name);
            break;
        case NETDEV_DOWN:
             if(dev && dev->name)
                 printk("inet[%s] is down\n",dev->name);
        default:
            break;

    }

    return NOTIFY_OK;
}

struct notifier_block inethandle={
    .notifier_call = test_inetaddr_event
};          
                       
struct notifier_block devhandle={
    .notifier_call = test_netdev_event
};

static int __init  my_init(void)
{   
    /*
     * 在netdev_chain通知鏈上註冊訊息塊 
     * netdev_chain通知鏈是核心中用於傳遞有關網路設備註冊狀態的通知資訊
     */
    register_netdevice_notifier(&devhandle);
	
    /*
     * 在inetaddr_chain通知鏈上註冊訊息塊
     * inetaddr_chain通知鏈是核心中用於傳遞有關本地介面上的ipv4地址的插入，刪除以及變更的通知資訊
     */
    register_inetaddr_notifier(&inethandle);
    return 0; 
}   

static void __exit my_exit(void)
{
    unregister_netdevice_notifier(&devhandle);
    unregister_inetaddr_notifier(&inethandle);
    return;
}


module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
