#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include <libmnl/libmnl.h>
#include <linux/genetlink.h>

#include "genlexample.h"

int main(int argc, char *argv[])
{
	char buf[getpagesize()];
	struct nlmsghdr *nlh;
	struct genlmsghdr *genl;
	struct mnl_socket *nl;
	int ret;
	unsigned int seq, oper, portid;
	unsigned int nl_flags = 0;

	if (argc < 2 || argc > 3) {
		printf("%s <genetlink family ID> [excl]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	if (argc == 3 && strcmp(argv[2], "excl") == 0)
		nl_flags |= NLM_F_EXCL;

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = atoi(argv[1]);
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | nl_flags;
	nlh->nlmsg_seq = seq = time(NULL);
	genl = mnl_nlmsg_put_extra_header(nlh, sizeof(struct genlmsghdr));
	genl->cmd = NLEX_CMD_UPD;

	mnl_attr_put_u32(nlh, NLE_MYVAR, time(NULL));

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

	ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
	if (ret == -1) {
		perror("mnl_cb_run");
		exit(EXIT_FAILURE);
	}
	mnl_socket_close(nl);

	printf("done\n");

	return 0;
}
