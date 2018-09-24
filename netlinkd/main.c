
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

int
main(int argc, char **argv)
{
	struct rtnl_handle rth;
	uint32_t groups = ~0U;
	if (rtnl_open(&rth, groups) < 0)
		return 1;

	if (rtnl_listen(&rth, dump_msg, NULL) < 0)
		return 2;
}

#if 0
int main(int argc, char** argv)
{
  int sock = netlink_sock_open();
  while (true) {
    uint8_t buf[1000];
    ssize_t ret = read(sock, buf, sizeof(buf));
    if (ret < 0) {
      return -1;
    }

    for (struct nlmsghdr* hdr = (struct nlmsghdr*)buf;
         NLMSG_OK(hdr, ret); hdr = NLMSG_NEXT(hdr, ret)) {
      netlink_msg_dump(stdout, hdr);
      printf("\n\n\n");
    }

  } /* while (true) */
  netlink_sock_close(sock);
}
#endif
