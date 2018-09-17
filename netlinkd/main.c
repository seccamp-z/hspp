
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "hexdump.h"

// netlink.h
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
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
  sa.nl_groups = 0xffffffff;
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

static const char* nlmsg_type_to_str(uint16_t type)
{
  switch (type) {
    case RTM_NEWLINK: return "RTM_NEWLINK";
    case RTM_DELLINK: return "RTM_DELLINK";
    case RTM_GETLINK: return "RTM_GETLINK";
    case RTM_NEWADDR: return "RTM_NEWADDR";
    case RTM_DELADDR: return "RTM_DELADDR";
    case RTM_GETADDR: return "RTM_GETADDR";
    case RTM_NEWROUTE: return "RTM_NEWROUTE";
    case RTM_DELROUTE: return "RTM_DELROUTE";
    case RTM_GETROUTE: return "RTM_GETROUTE";
    case RTM_NEWNEIGH: return "RTM_NEWNEIGH";
    case RTM_DELNEIGH: return "RTM_DELNEIGH";
    case RTM_GETNEIGH: return "RTM_GETNEIGH";
    case RTM_NEWRULE: return "RTM_NEWRULE";
    case RTM_DELRULE: return "RTM_DELRULE";
    case RTM_GETRULE: return "RTM_GETRULE";
    case RTM_NEWQDISC: return "RTM_NEWQDISC";
    case RTM_DELQDISC: return "RTM_DELQDISC";
    case RTM_GETQDISC: return "RTM_GETQDISC";
    case RTM_NEWTCLASS: return "RTM_NEWTCLASS";
    case RTM_DELTCLASS: return "RTM_DELTCLASS";
    case RTM_GETTCLASS: return "RTM_GETTCLASS";
    case RTM_NEWTFILTER: return "RTM_NEWTFILTER";
    case RTM_DELTFILTER: return "RTM_DELTFILTER";
    case RTM_GETTFILTER: return "RTM_GETTFILTER";
    default:
      fprintf(stderr, "%s: unknown\n", __func__);
      exit(1);
  }
}

void netlink_msg_dump(FILE* fp, const struct nlmsghdr* hdr)
{
  printf("nlmsg_len  : %u\n", hdr->nlmsg_len  );
  printf("nlmsg_type : %u (%s)\n", hdr->nlmsg_type, nlmsg_type_to_str(hdr->nlmsg_type));
  printf("nlmsg_flags: %u\n", hdr->nlmsg_flags);
  printf("nlmsg_seq  : %u\n", hdr->nlmsg_seq  );
  printf("nlmsg_pid  : %u\n", hdr->nlmsg_pid  );
  hexdump(fp, hdr, hdr->nlmsg_len);
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
      printf("msg\n");
      netlink_msg_dump(stdout, hdr);
    }

    /* netlink_msg_dump(stdout, buf, ret); */
  }
  netlink_sock_close(sock);
}

