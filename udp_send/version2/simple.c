/*
 *   run this first, then insmod ./simple.ko
 *   tcpdump -lvnn dst 192.168.1.253 -A
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/socket.h>
#include <net/sock.h>

struct socket     *sock;
extern struct net init_net;

unsigned char buffer[10]= "hello\n";

static int ker_send_udp(char* ip_addr, unsigned char * data, size_t len )
{
    int ret;
    int i;
    u32 remote_ip = in_aton(ip_addr);
  
    struct sockaddr_in sin = {
        .sin_family = AF_INET,
        .sin_port = htons(65530),
        .sin_addr = {.s_addr = remote_ip}
    };
 
    struct kvec iov = {.iov_base = (void *)data, .iov_len = len};
    struct msghdr udpmsg;

    udpmsg.msg_name = (void *)&sin;
    udpmsg.msg_namelen = sizeof(sin);
    udpmsg.msg_control = NULL;
    udpmsg.msg_controllen = 0;
    udpmsg.msg_flags=0;

    for(i = 0; i<10; i++)
        ret = kernel_sendmsg(sock, &udpmsg, &iov, 1, len);
   
    return 0;
}

static int my_init (void)
{
    sock_create_kern (&init_net, PF_INET, SOCK_DGRAM,IPPROTO_UDP, &sock);
    ker_send_udp("192.168.1.253", buffer, 10);
    return 0;
}

static void my_exit (void)
{   
    sock_release (sock);
}

module_init (my_init);
module_exit (my_exit);
MODULE_LICENSE ("GPL");
