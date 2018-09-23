
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "libnetlink.h"
#include "netlink_dump.h"

static int
dump_msg(const struct sockaddr_nl *who,
         struct rtnl_ctrl_data *ctrl,
		     struct nlmsghdr *n, void *arg)
{
  netlink_msg_dump(stdout, n);
	return 0;
}

/* static int */
/* dump_msg2(const struct sockaddr_nl *who, */
/* 		     struct nlmsghdr *n, void *arg) */
/* { */
/* 	return dump_msg(who, NULL, n, arg); */
/* } */

int
main(int argc, char **argv)
{
	struct rtnl_handle rth;
	uint32_t groups = ~0U;
	if (rtnl_open(&rth, groups) < 0)
		return 1;

	if (rtnl_wilddump_request(&rth, AF_UNSPEC, RTM_GETLINK) < 0) {
		perror("Cannot send dump request");
		return 1;
	}

	/* if (rtnl_dump_filter(&rth, dump_msg2, NULL) < 0) { */
	/* 	fprintf(stderr, "Dump terminated\n"); */
	/* 	return 1; */
	/* } */

	if (rtnl_listen(&rth, dump_msg, NULL) < 0)
		return 2;
}

