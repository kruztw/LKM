// code: https://stackoverflow.com/questions/62273254/how-to-write-linux-kernel-module-that-is-a-new-instance-in-each-network-namespac
// concept: https://blog.csdn.net/maryfei/article/details/81252800

#include <net/sock.h>
#include <net/netns/generic.h>
#include <net/net_namespace.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/pid_namespace.h>


struct ns_data {
  char message[0x10];
};

static unsigned int net_id;


static int __net_init my_ns_init(struct net *net)
{
  struct ns_data *data = net_generic(net, net_id);
  strcpy(data->message, "hello");
  return 0;
}

static void __net_exit my_ns_exit(struct net *net)
{
  struct ns_data *data = net_generic(net, net_id);
  printk("message: %s\n", data->message);
}

static struct pernet_operations net_ops __net_initdata = {
  .init = my_ns_init,
  .exit = my_ns_exit,
  .id = &net_id,
  .size = sizeof(struct ns_data),
};

static int __init my_init(void)
{
  // allocate .size chunk  maintained in struct net.
  // the id will be stored in .id, which is used to get this chunk ( by using net_generic )
  register_pernet_subsys(&net_ops);
  return 0;
}

static void __exit my_exit(void)
{
  unregister_pernet_subsys(&net_ops);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
