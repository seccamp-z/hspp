
#ifndef _NETLINK_SOCKET_H_
#define _NETLINK_SOCKET_H_

#include <unistd.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

int
netlink_sock_open(void)
{
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock < 0) {
    fprintf(stderr, "socket: open netlink socket\n");
    exit(1);
  }

  struct sockaddr_nl sa;
  sa.nl_family = AF_NETLINK;
  sa.nl_groups = ~0U;
  int ret = bind(sock, (struct sockaddr*)&sa, sizeof(sa));
  if (ret < 0) {
    fprintf(stderr, "bind: open netlink socket\n");
    exit(1);
  }

  return sock;
}

void
netlink_sock_close(int sock)
{
  close(sock);
}

#endif /* _NETLINK_SOCKET_H_ */

