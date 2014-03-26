#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include <libmnl/libmnl.h>
#include <linux/genetlink.h>

#include "genlexample.h"

static int data_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NLE_MYVAR) < 0)
		return MNL_CB_OK;

	switch(type) {
        case NLE_MYVAR:
                if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	}
	tb[NLE_MYVAR] = attr;
	return MNL_CB_OK;
}

static int cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[NLE_MAX];
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*genl), data_attr_cb, tb);
	if (tb[NLE_MYVAR])
		printf("myvar=%d\n", mnl_attr_get_u32(tb[NLE_MYVAR]));

	return MNL_CB_OK;
}

int main(int argc, char *argv[])
{
	char buf[getpagesize()];
	struct nlmsghdr *nlh;
	struct genlmsghdr *genl;
	struct mnl_socket *nl;
	int ret;
	unsigned int seq, oper, portid;

	if (argc != 2) {
		printf("%s <genetlink family ID>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = atoi(argv[1]);
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	nlh->nlmsg_seq = seq = time(NULL);
	genl = mnl_nlmsg_put_extra_header(nlh, sizeof(struct genlmsghdr));
	genl->cmd = NLEX_CMD_GET;

	nl = mnl_socket_open(NETLINK_GENERIC);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}
	ret = mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID);
	if (ret == -1) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}
	portid = mnl_socket_get_portid(nl);

	ret = mnl_socket_sendto(nl, nlh, nlh->nlmsg_len);
	if (ret == -1) {
		perror("mnl_socket_send");
		exit(EXIT_FAILURE);
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret == -1) {
		perror("mnl_socket_recvfrom");
		exit(EXIT_FAILURE);
	}

	ret = mnl_cb_run(buf, ret, seq, portid, cb, NULL);
	if (ret == -1) {
		perror("mnl_cb_run");
		exit(EXIT_FAILURE);
	}
	mnl_socket_close(nl);

	printf("done\n");

	return 0;
}
