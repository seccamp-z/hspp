
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "hexdump.h"

// netlink.h
#include <unistd.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "netlink_dump.h"

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

void
netlink_msg_dump(FILE* fp, const struct nlmsghdr* hdr)
{
  printf("nlmsg_len: %u\n", hdr->nlmsg_len  );
  printf("nlmsg_type: %u (%s)\n", hdr->nlmsg_type, nlmsg_type_to_str(hdr->nlmsg_type));
  printf("nlmsg_flags: %u\n", hdr->nlmsg_flags);
  printf("nlmsg_seq: %u\n", hdr->nlmsg_seq  );
  printf("nlmsg_pid: %u\n", hdr->nlmsg_pid  );

  uint16_t type = hdr->nlmsg_type;
  switch (type) {
    case RTM_NEWLINK:
    case RTM_DELLINK:
    case RTM_GETLINK:
      netlink_link_msg_dump(fp, hdr);
      break;
    case RTM_NEWADDR:
    case RTM_DELADDR:
    case RTM_GETADDR:
      netlink_addr_msg_dump(fp, hdr);
      break;
    case RTM_NEWROUTE:
    case RTM_DELROUTE:
    case RTM_GETROUTE:
      netlink_route_msg_dump(fp, hdr);
      break;
    case RTM_NEWNEIGH:
    case RTM_DELNEIGH:
    case RTM_GETNEIGH:
      netlink_neigh_msg_dump(fp, hdr);
      break;
    default:
      fprintf(stderr, "%s: unknown\n", __func__);
      hexdump(stderr, hdr, hdr->nlmsg_len);
      exit(1);
      break;
  }
}

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

