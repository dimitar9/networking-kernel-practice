#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <net/net_namespace.h>

#ifndef NETLINK_EXAMPLE
#define NETLINK_EXAMPLE 21
#endif

#define NLEX_GRP_MAX	0

static struct sock *nlsk;
static int myvar;

static void
nl_callback(struct sk_buff *skb)
{
   pr_err("the current value is %d\n", myvar);
}

static int __init nlexample_init(void)
{

    pr_err("init nlexample module\n");
	struct netlink_kernel_cfg cfg = {
		.groups = NLEX_GRP_MAX,
		.input	= nl_callback,
	};

    pr_err("init nlexample module before create\n");
	nlsk = netlink_kernel_create(&init_net, NETLINK_EXAMPLE, &cfg);
	if (nlsk == NULL) {
		pr_err("Can't create netlink example\n");
		return -ENOMEM;
	}
    else
    {
        pr_err("nlsk is %d\n", nlsk);
    }
	return 0;
}

void __exit nlexample_exit(void)
{
	netlink_kernel_release(nlsk);
    pr_err("eixt nlexample module\n");
}

module_init(nlexample_init);
module_exit(nlexample_exit);

MODULE_AUTHOR("Pablo Neira Ayuso <pablo@netfilter.org>");
MODULE_LICENSE("GPL");
