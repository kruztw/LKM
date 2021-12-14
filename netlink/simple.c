// nlink msg structure: http://3.bp.blogspot.com/_l7WuTDxFApo/S_jb8nlqvAI/AAAAAAAAAhw/XZrfKCaWKMc/s1600/netlink_message_layout_and_macro_interaction.png

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <net/sock.h>
#include <linux/netlink.h>

#define NETLINK_TEST        30        /* 0 ~ 32 , linux uses 0~21 (maybe) */
#define USER_PORT          100

MODULE_LICENSE("GPL");

struct sock *nlsk = NULL;   // netlink socket
extern struct net init_net;

int send_msg(char *pbuf, uint16_t len)
{
    struct sk_buff *nl_skb;
    struct nlmsghdr *nlh;
    
    nl_skb = nlmsg_new(len, GFP_ATOMIC);
    if (!nl_skb) {
        printk("netlink alloc failure\n");
        return -1;
    }
    
    /*
     * nlmsg_put - Add a new netlink message to an skb
     * @skb: socket buffer to store message in
     * @portid: netlink PORTID of requesting application
     * @seq: sequence number of message
     * @type: message type
     * @payload: length of message payload
     * @flags: message flags
     *
     * Returns NULL if the tailroom of the skb is insufficient to store
     * the message header and payload.
     */
    nlh = nlmsg_put(nl_skb, 0, 0, NETLINK_TEST, len, 0);
    if (nlh == NULL) {
        printk("nlmsg_put failaure \n");
        nlmsg_free(nl_skb);
        return -1;
    }
    
    memcpy(nlmsg_data(nlh), pbuf, len);

    /* netlink_unicast(struct sock *ssk, struct sk_buff *skb, u32 portid, int nonblock) */
    return netlink_unicast(nlsk, nl_skb, USER_PORT, MSG_DONTWAIT);
}

static void rcv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    char *umsg = NULL;
    char *kmsg = "hello users!!!";

    /* lmsg_total_size(payload) : length of message w/ padding , payload = 0 means header only */
    if (skb->len >= nlmsg_total_size(0)) {
        nlh = nlmsg_hdr(skb);
        umsg = NLMSG_DATA(nlh);
        if (umsg) {
            printk("kernel recv from user: %s\n", umsg);
            send_msg(kmsg, strlen(kmsg));
        }
    }
}

struct netlink_kernel_cfg cfg = { 
    .input  = rcv_msg, /* set recv callback */
};
  
int test_netlink_init(void)
{
    /* netlink_kernel_create(struct net *net, int unit, struct netlink_kernel_cfg *cfg) */
    nlsk = (struct sock *)netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
    if (nlsk == NULL) {   
        printk("netlink_kernel_create failed !\n");
        return -1; 
    } 
    return 0;
}

void test_netlink_exit(void)
{
    if (nlsk) {
        netlink_kernel_release(nlsk); /* release ..*/
        nlsk = NULL;
    }   
    printk("test_netlink_exit!\n");
}

module_init(test_netlink_init);
module_exit(test_netlink_exit);
