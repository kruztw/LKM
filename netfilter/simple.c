#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

static struct nf_hook_ops *nfho = NULL;

unsigned int hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    printk(KERN_INFO "hook\n");
    return NF_DROP;
}

static int __init my_init(void)
{
    nfho = (struct nf_hook_ops*) kcalloc(1, sizeof(struct nf_hook_ops), GFP_KERNEL);

    nfho->hook = (nf_hookfn*) hook_func;
    nfho->hooknum = NF_INET_PRE_ROUTING;
    nfho->pf = PF_INET;
    nfho->priority = NF_IP_PRI_FIRST;

    nf_register_net_hook(&init_net, nfho);
    return 0;
}

static void __exit my_exit(void)
{
    nf_unregister_net_hook(&init_net, nfho);
    kfree(nfho);
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");

