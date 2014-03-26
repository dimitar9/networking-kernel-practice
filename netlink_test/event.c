#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <libmnl/libmnl.h>

#include "nlexample.h"

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
	struct nlattr *tb[NLE_MAX+1] = {};

	mnl_attr_parse(nlh, 0, parse_attr_cb, tb);
	if (tb[NLE_MYVAR])
		printf("myvar=%u\n", mnl_attr_get_u32(tb[NLE_MYVAR]));

	return MNL_CB_OK;
}

int main()
{
	struct mnl_socket *nl;
	char buf[getpagesize()];
	int ret;

	nl = mnl_socket_open(NETLINK_EXAMPLE);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, NLEX_GRP_MYVAR, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, 0, 0, data_cb, NULL);
		if (ret <= 0)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1) {
		perror("error");
		exit(EXIT_FAILURE);
	}

	mnl_socket_close(nl);

	return 0;
}
