#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <libmnl/libmnl.h>
#include <linux/genetlink.h>

static int parse_attr_mcast_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = (const struct nlattr **)data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTRL_ATTR_MCAST_GRP_MAX) < 0) {
		perror("mnl_attr_type_valid");
		return MNL_CB_ERROR;
	}

	switch(type) {
	case CTRL_ATTR_MCAST_GRP_NAME:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	case CTRL_ATTR_MCAST_GRP_ID:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static void
genl_parse_one_mc_group(struct nlattr *pos)
{
	struct nlattr *tb[CTRL_ATTR_MCAST_GRP_MAX+1] = {};

	mnl_attr_parse_nested(pos, parse_attr_mcast_cb, tb);
	if (tb[CTRL_ATTR_MCAST_GRP_NAME] &&
	    tb[CTRL_ATTR_MCAST_GRP_ID]) {
		printf("\"%s\" -> %d\n",
			mnl_attr_get_str(tb[CTRL_ATTR_MCAST_GRP_NAME]),
			mnl_attr_get_u32(tb[CTRL_ATTR_MCAST_GRP_ID]));
	}
}

static void
genl_parse_mc_groups(struct nlattr *nested)
{
	struct nlattr *pos;
	const char *mcast_grp_name;
	int mcast_grp_id;

	mnl_attr_for_each_nested(pos, nested)
		genl_parse_one_mc_group(pos);
}

static int parse_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = (const struct nlattr **)data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTRL_ATTR_MAX) < 0) {
		perror("mnl_attr_type_valid");
		return MNL_CB_ERROR;
	}
	switch(type) {
	case CTRL_ATTR_FAMILY_ID:
		if (mnl_attr_validate(attr, MNL_TYPE_U16) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	case CTRL_ATTR_MCAST_GROUPS:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0) {
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
	struct nlattr *tb[CTRL_ATTR_MAX+1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*genl), parse_attr_cb, tb);
	if (tb[CTRL_ATTR_FAMILY_ID]) {
		printf("family-id: %d\n",
			mnl_attr_get_u16(tb[CTRL_ATTR_FAMILY_ID]));
	}
	if (tb[CTRL_ATTR_MCAST_GROUPS]) {
		printf("multicast -> id\n");
		genl_parse_mc_groups(tb[CTRL_ATTR_MCAST_GROUPS]);
	}
	return MNL_CB_OK;
}

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[getpagesize()];
	struct nlmsghdr *nlh;
	struct genlmsghdr *genl;
	int ret;
	unsigned int seq, portid;

	if (argc != 2) {
		printf("%s [family name]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type	= GENL_ID_CTRL;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	nlh->nlmsg_seq = seq = time(NULL);

	genl = mnl_nlmsg_put_extra_header(nlh, sizeof(struct genlmsghdr));
	genl->cmd = CTRL_CMD_GETFAMILY;
	genl->version = 0;

	mnl_attr_put_strz(nlh, CTRL_ATTR_FAMILY_NAME, argv[1]);

	nl = mnl_socket_open(NETLINK_GENERIC);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}
	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		perror("mnl_socket_send");
		exit(EXIT_FAILURE);
	}

	while (1) {
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
		if (ret == -1) {
			perror("mnl_socket_recvfrom");
			break;
		}
		ret = mnl_cb_run(buf, ret, seq, portid, data_cb, NULL);
		if (ret == -1) {
			perror("mnl_cb_run");
			break;
		} else if (ret == 0)
			break;
	}
	mnl_socket_close(nl);

	return 0;
}
