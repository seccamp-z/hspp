
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

static const char*
rtn_type_to_str(uint8_t type)
{
  switch (type) {
    case RTN_UNSPEC: return "RTN_UNSPEC";
    case RTN_UNICAST: return "RTN_UNICAST";
    case RTN_LOCAL: return "RTN_LOCAL";
    case RTN_BROADCAST: return "RTN_BROADCAST";
    case RTN_ANYCAST: return "RTN_ANYCAST";
    case RTN_MULTICAST: return "RTN_MULTICAST";
    case RTN_BLACKHOLE: return "RTN_BLACKHOLE";
    case RTN_UNREACHABLE: return "RTN_UNREACHABLE";
    case RTN_PROHIBIT: return "RTN_PROHIBIT";
    case RTN_THROW: return "RTN_THROW";
    case RTN_NAT: return "RTN_NAT";
    case RTN_XRESOLVE: return "RTN_XRESOLVE";
    default:
      fprintf(stderr, "%s: unknown(%u)\n", __func__, type);
      exit(1);
  }
}

static const char*
ifa_family_to_str(uint8_t family)
{
  switch (family) {
    case AF_UNSPEC    : return "AF_UNSPEC";
    case AF_UNIX      : return "AF_UNIX";
    case AF_INET      : return "AF_INET";
    case AF_AX25      : return "AF_AX25";
    case AF_IPX       : return ";AF_IPX";
    case AF_APPLETALK : return "AF_APPLETALK";
    case AF_NETROM    : return "AF_NETROM";
    case AF_BRIDGE    : return "AF_BRIDGE";
    case AF_ATMPVC    : return "AF_ATMPVC";
    case AF_X25       : return "AF_X25";
    case AF_INET6     : return "AF_INET6";
    case AF_ROSE      : return "AF_ROSE";
    case AF_DECnet    : return "AF_DECnet";
    case AF_NETBEUI   : return "AF_NETBEUI";
    case AF_SECURITY  : return "AF_SECURITY";
    case AF_KEY       : return "AF_KEY";
    case AF_NETLINK   : return "AF_NETLINK";
    case AF_PACKET    : return "AF_PACKET";
    case AF_ASH       : return "AF_ASH";
    case AF_ECONET    : return "AF_ECONET";
    case AF_ATMSVC    : return "AF_ATMSVC";
    case AF_RDS       : return "AF_RDS";
    case AF_SNA       : return "AF_SNA";
    case AF_IRDA      : return "AF_IRDA";
    case AF_PPPOX     : return "AF_PPPOX";
    case AF_WANPIPE   : return "AF_WANPIPE";
    case AF_LLC       : return "AF_LLC";
    case AF_IB        : return "AF_IB";
    case AF_MPLS      : return "AF_MPLS";
    case AF_CAN       : return "AF_CAN";
    case AF_TIPC      : return "AF_TIPC";
    case AF_BLUETOOTH : return "AF_BLUETOOTH";
    case AF_IUCV      : return "AF_IUCV";
    case AF_RXRPC     : return "AF_RXRPC";
    case AF_ISDN      : return "AF_ISDN";
    case AF_PHONET    : return "AF_PHONET";
    case AF_IEEE802154: return "AF_IEEE802154";
    case AF_CAIF      : return "AF_CAIF";
    case AF_ALG       : return "AF_ALG";
    case AF_NFC       : return "AF_NFC";
    case AF_VSOCK     : return "AF_VSOCK";
    case AF_MAX       : return "AF_MAX";
    default:
      fprintf(stderr, "%s: unknown\n", __func__);
      exit(1);
  }
}

static void
netlink_link_msg_dump(FILE* fp, const struct nlmsghdr* hdr)
{
  printf("this is link msg\n");
  hexdump(fp, hdr, hdr->nlmsg_len);
}

static const char*
rta_type_to_str(uint16_t type)
{
  switch (type) {
    case IFLA_UNSPEC         : return "IFLA_UNSPEC";
    case IFLA_ADDRESS        : return "IFLA_ADDRESS";
    case IFLA_BROADCAST      : return "IFLA_BROADCAST";
    case IFLA_IFNAME         : return "IFLA_IFNAME";
    case IFLA_MTU            : return "IFLA_MTU ";
    case IFLA_LINK           : return "IFLA_LINK ";
    case IFLA_QDISC          : return "IFLA_QDISC";
    case IFLA_STATS          : return "IFLA_STATS";
    case IFLA_COST           : return "IFLA_COST";
    case IFLA_PRIORITY       : return "IFLA_PRIORITY";
    case IFLA_MASTER         : return "IFLA_MASTER";
    case IFLA_WIRELESS       : return "IFLA_WIRELESS";
    case IFLA_PROTINFO       : return "IFLA_PROTINFO";
    case IFLA_TXQLEN         : return "IFLA_TXQLEN";
    case IFLA_MAP            : return "IFLA_MAP";
    case IFLA_WEIGHT         : return "IFLA_WEIGHT";
    case IFLA_OPERSTATE      : return "IFLA_OPERSTATE";
    case IFLA_LINKMODE       : return "IFLA_LINKMODE";
    case IFLA_LINKINFO       : return "IFLA_LINKINFO";
    case IFLA_NET_NS_PID     : return "IFLA_NET_NS_PID";
    case IFLA_IFALIAS        : return "IFLA_IFALIAS";
    case IFLA_NUM_VF         : return "IFLA_NUM_VF";
    case IFLA_VFINFO_LIST    : return "IFLA_VFINFO_LIST";
    case IFLA_STATS64        : return "IFLA_STATS64";
    case IFLA_VF_PORTS       : return "IFLA_VF_PORTS";
    case IFLA_PORT_SELF      : return "IFLA_PORT_SELF";
    case IFLA_AF_SPEC        : return "IFLA_AF_SPEC";
    case IFLA_GROUP          : return "IFLA_GROUP";
    case IFLA_NET_NS_FD      : return "IFLA_NET_NS_FD";
    case IFLA_EXT_MASK       : return "IFLA_EXT_MASK";
    case IFLA_PROMISCUITY    : return "IFLA_PROMISCUITY";
    case IFLA_NUM_TX_QUEUES  : return "IFLA_NUM_TX_QUEUES";
    case IFLA_NUM_RX_QUEUES  : return "IFLA_NUM_RX_QUEUES";
    case IFLA_CARRIER        : return "IFLA_CARRIER";
    case IFLA_PHYS_PORT_ID   : return "IFLA_PHYS_PORT_ID";
    case IFLA_CARRIER_CHANGES: return "IFLA_CARRIER_CHANGES";
    case IFLA_PHYS_SWITCH_ID : return "IFLA_PHYS_SWITCH_ID";
    case IFLA_LINK_NETNSID   : return "IFLA_LINK_NETNSID";
    case IFLA_PHYS_PORT_NAME : return "IFLA_PHYS_PORT_NAME";
    case IFLA_PROTO_DOWN     : return "IFLA_PROTO_DOWN";
    default:
      fprintf(stderr, "%s: unknown(%u)\n", __func__, type);
      exit(1);
      break;
  }
}

static const char*
rta_to_str(const struct rtattr* rta, char* str, size_t len)
{
  char dstr[100];
  memset(dstr, 0, sizeof(dstr));
  uint8_t* ptr = (uint8_t*)(rta + 1);
  for (size_t i=0; i<rta->rta_len-sizeof(struct rtattr); i++) {
    char sub_str[10];
    snprintf(sub_str, sizeof(sub_str), "%02x", ptr[i]);
    strncat(dstr, sub_str, sizeof(dstr));
  }
  memset(str, 0x0, len);
  snprintf(str, len, "%s data=%s",
      rta_type_to_str(rta->rta_type), dstr);
  return str;
}

static void
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

static void
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

static void
netlink_neigh_msg_dump(FILE* fp, const struct nlmsghdr* hdr)
{
  printf("this is neigh msg\n");
  hexdump(fp, hdr, hdr->nlmsg_len);
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

