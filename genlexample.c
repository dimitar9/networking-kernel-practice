#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/genetlink.h>
#include <net/genetlink.h>
#include <net/sock.h>

#include "genlexample.h"

static int myvar;

static struct genl_family genl_example_family = {
	.id = GENL_ID_GENERATE,
	.name = "nlex",
	.hdrsize = 0,
	.version = 1,
	.maxattr = NLE_MAX,
};

static struct genl_multicast_group genl_example_mc = {
	.name		= "example",
};

static int
genl_get_myvar(struct sk_buff *skb, struct genl_info *info)
{
	struct sk_buff *msg;
	void *hdr;

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (msg == NULL)
		return -ENOMEM;

	hdr = genlmsg_put(msg, info->snd_portid, info->snd_seq,
			  &genl_example_family, 0, NLEX_CMD_UPD);
	if (hdr == NULL)
		goto nlmsg_failure;

	if (nla_put_u32(msg, NLE_MYVAR, myvar))
		goto nla_put_failure;

	genlmsg_end(msg, hdr);

	genlmsg_unicast(sock_net(skb->sk), msg, info->snd_portid);

	return 0;

nlmsg_failure:
nla_put_failure:
	genlmsg_cancel(msg, hdr);
	kfree_skb(msg);
	return -ENOBUFS;
}

static int
genl_upd_myvar(struct sk_buff *skb, struct genl_info *info)
{
	struct sk_buff *msg;
	void *hdr;

	if (!info->attrs[NLE_MYVAR])
		return -EINVAL;

	myvar = nla_get_u32(info->attrs[NLE_MYVAR]);

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (msg == NULL)
		return -ENOMEM;

	hdr = genlmsg_put(msg, 0, 0, &genl_example_family, 0, NLEX_CMD_UPD);
	if (hdr == NULL)
		goto nlmsg_failure;

	if (nla_put_u32(msg, NLE_MYVAR, myvar))
		goto nla_put_failure;

	genlmsg_end(msg, hdr);

	genlmsg_multicast(msg, 0, genl_example_mc.id, GFP_KERNEL);
	return 0;

nlmsg_failure:
nla_put_failure:
	genlmsg_cancel(msg, hdr);
	kfree_skb(msg);
	return -ENOBUFS;
}

static struct genl_ops genl_example_ops[] = {
	{
		.cmd = NLEX_CMD_GET,
		.doit = genl_get_myvar,
	},
	{
		.cmd = NLEX_CMD_UPD,
		.doit = genl_upd_myvar,
	},
};

static int __init nlexample_init(void)
{
	int i, ret = -EINVAL;

	ret = genl_register_family(&genl_example_family);
	if (ret < 0)
		goto err;

	for (i = 0; i < ARRAY_SIZE(genl_example_ops); i++) {
		ret = genl_register_ops(&genl_example_family,
					&genl_example_ops[i]);
		if (ret < 0)
			goto err_unregister;
	}

	ret = genl_register_mc_group(&genl_example_family, &genl_example_mc);
	if (ret < 0)
		goto err_unregister;

	return ret;

err_unregister:
	genl_unregister_family(&genl_example_family);
err:
	return ret;
}

void __exit nlexample_exit(void)
{
	genl_unregister_family(&genl_example_family);
}

module_init(nlexample_init);
module_exit(nlexample_exit);

MODULE_LICENSE("GPL");
