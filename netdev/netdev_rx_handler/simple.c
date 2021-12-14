#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/net.h>
#include <linux/inetdevice.h>
#include <linux/in.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <asm/unaligned.h>
#include <linux/kthread.h>
#include <linux/netdevice.h>

extern struct net init_net;

MODULE_LICENSE("GPL");

char *ifname = "lo";
module_param(ifname, charp, 0644);
MODULE_PARM_DESC(ifname, "Send packets from which net device");

struct net_device *dev;

static rx_handler_result_t netdev_frame_hook(struct sk_buff **pskb)
{
    printk("hello\n");
    return RX_HANDLER_PASS;
}

static int __init my_init(void)
{
    int err = 0;
    struct socket *sock;
    struct net *net;

    err = sock_create_kern(&init_net, PF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock);
    if (err < 0) {
        printk(KERN_ALERT "UDP create sock err, err=%d\n", err);
        release_sock(sock->sk);
        return -1;
    }
    sock->sk->sk_reuse = 1;

    net = sock_net(sock->sk);
    dev = __dev_get_by_name(net, ifname);

    err = netdev_rx_handler_register(dev, netdev_frame_hook, NULL);
    release_sock(sock->sk);
    return 0;
}

static void __exit my_exit(void)
{
    netdev_rx_handler_unregister(dev);
    return;
}


module_init(my_init);
module_exit(my_exit);
