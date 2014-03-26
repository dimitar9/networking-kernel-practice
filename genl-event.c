#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <libmnl/libmnl.h>
#include <linux/genetlink.h>

#include "genlexample.h"

static int parse_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = (const struct nlattr **)data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NLE_MAX) < 0) {
		perror("mnl_attr_type_valid");
		return MNL_CB_ERROR;
	}

	switch(type) {
	case NLE_MYVAR:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int data_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[NLE_MAX+1];
	struct nlattr *attr;

	mnl_attr_parse(nlh, sizeof(struct genlmsghdr), parse_attr_cb, tb);
	if (tb[NLE_MYVAR])
		printf("myvar=%u\n", mnl_attr_get_u32(tb[NLE_MYVAR]));

	return MNL_CB_OK;
}

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[getpagesize()];
	struct nlmsghdr *nlh = (struct nlmsghdr *) buf;
	int ret, grp;

	if (argc != 2) {
		printf("%s [genetlink multicast group]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

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

	grp = atoi(argv[1]);
	mnl_socket_setsockopt(nl, NETLINK_ADD_MEMBERSHIP, &grp, sizeof(grp));

	while (1) {
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
		if (ret == -1) {
			perror("mnl_socket_recvfrom");
			exit(EXIT_FAILURE);
		}
		ret = mnl_cb_run(buf, ret, 0, 0, data_cb, NULL);
		if (ret == -1) {
			perror("mnl_cb_run");
			exit(EXIT_FAILURE);
		}
	}

	mnl_socket_close(nl);

	return 0;
}
