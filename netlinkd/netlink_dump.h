
#ifndef _NETLINK_DUMP_H_
#define _NETLINK_DUMP_H_

// netlink.h
#include <unistd.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "netlink_types.h"
#include "hexdump.h"

inline static void
netlink_link_msg_dump(FILE* fp, const struct nlmsghdr* hdr)
{
  printf("this is link msg\n");
  hexdump(fp, hdr, hdr->nlmsg_len);
}

inline static void
netlink_addr_msg_dump(FILE* fp, const struct nlmsghdr* hdr)
{
  char strbuf[256];
  struct ifaddrmsg* ifa = (struct ifaddrmsg*)(hdr + 1);
  printf("ifa_family: %u (%s)\n",
      ifa->ifa_family, ifa_family_to_str(ifa->ifa_family));
  printf("ifa_prefixlen: %u\n", ifa->ifa_prefixlen);
  printf("ifa_flags: %u\n", ifa->ifa_flags);
  printf("ifa_scope: %u\n", ifa->ifa_scope    );
  printf("ifa_index: %d (%s)\n", ifa->ifa_index,
      if_indextoname(ifa->ifa_index, strbuf));

  size_t rta_len = IFA_PAYLOAD(hdr);
  size_t i=0;
  for (struct rtattr* rta = IFA_RTA(ifa);
       RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
    char str[512];
    printf("attr[%zd]: %s\n", i++, rta_to_str(rta, str, sizeof(str)));
  }
}

inline static void
netlink_route_msg_dump(FILE* fp, const struct nlmsghdr* hdr)
{
  struct rtmsg* rtm = (struct rtmsg*)(hdr + 1);
  printf("rtm_family: %u (%s)\n",
      rtm->rtm_family, ifa_family_to_str(rtm->rtm_family));
  printf("rtm_dst_len: %u\n", rtm->rtm_dst_len);
  printf("rtm_src_len: %u\n", rtm->rtm_src_len);
  printf("rtm_tos: %u\n", rtm->rtm_tos);
  printf("rtm_table: %u\n", rtm->rtm_table);
  printf("rtm_protocol: %u\n", rtm->rtm_protocol);
  printf("rtm_scope: %u\n", rtm->rtm_scope);
  printf("rtm_type: %u (%s)\n", rtm->rtm_type,
      rtn_type_to_str(rtm->rtm_type));
  printf("rtm_flags: %u\n", rtm->rtm_flags);

  size_t i=0;
  size_t rta_len = IFA_PAYLOAD(hdr);
  for (struct rtattr* rta = RTM_RTA(rtm);
       RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
    char str[512];
    printf("attr[%zd]: %s\n", i++, rta_to_str(rta, str, sizeof(str)));
  }
}

inline static void
netlink_neigh_msg_dump(FILE* fp, const struct nlmsghdr* hdr)
{
  printf("this is neigh msg\n");
  hexdump(fp, hdr, hdr->nlmsg_len);
}

#endif /* _NETLINK_DUMP_H_ */

