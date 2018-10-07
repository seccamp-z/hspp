
#ifndef _NETLINK_RTATTR_H_
#define _NETLINK_RTATTR_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if_arp.h>
#include "flags.h"
#include "hexdump.h"

inline static const char*
rta_to_str(const struct rtattr* rta, char* str, size_t len)
{
  switch (rta->rta_type) {
    case IFLA_IFNAME:
    {
      snprintf(str, len, "%s <%s>",
          rta_type_to_str(rta->rta_type), (const char*)(rta+1));
      return str;
    }
    case IFLA_ADDRESS:
    case IFLA_BROADCAST:
    {
      char substr[1000];
      memset(substr, 0x0, sizeof(substr));
      size_t data_len = rta->rta_len-sizeof(struct rtattr);
      uint8_t* data_ptr = (uint8_t*)(rta + 1);
      for (size_t i=0; i<data_len; i++) {
        char subsubstr[100];
        snprintf(subsubstr, sizeof(subsubstr), "%02x:", data_ptr[i]);
        strncat(substr, subsubstr, strlen(subsubstr));
      }
      snprintf(str, len, "%s <%s>",
          rta_type_to_str(rta->rta_type), substr);
      return str;
    }
    case IFLA_STATS64:
    {
      struct rtnl_link_stats64* st = (struct rtnl_link_stats64*)(rta + 1);
      snprintf(str, len, "%s \n"
        "\trx_packets:%llu tx_packets:%llu rx_bytes:%llu tx_bytes:%llu \n"
        "\trx_errors:%llu tx_errors:%llu rx_dropped:%llu tx_dropped:%llu \n"
        "\tmulticast:%llu collisions:%llu rx_length_errors:%llu rx_over_errors:%llu \n"
        "\trx_crc_errors:%llu rx_frame_errors:%llu rx_fifo_errors:%llu \n"
        "\trx_missed_errors:%llu tx_aborted_errors:%llu tx_carrier_errors:%llu \n"
        "\ttx_fifo_errors:%llu " "tx_heartbeat_errors:%llu tx_window_errors:%llu \n"
        "\trx_compressed:%llu tx_compressed:%llu \n",
        rta_type_to_str(rta->rta_type),
        st->rx_packets, st->tx_packets, st->rx_bytes, st->tx_bytes,
        st->rx_errors, st->tx_errors, st->rx_dropped, st->tx_dropped,
        st->multicast, st->collisions, st->rx_length_errors, st->rx_over_errors,
        st->rx_crc_errors, st->rx_frame_errors, st->rx_fifo_errors,
        st->rx_missed_errors, st->tx_aborted_errors, st->tx_carrier_errors,
        st->tx_fifo_errors, st->tx_heartbeat_errors, st->tx_window_errors,
        st->rx_compressed, st->tx_compressed);
      return str;
    }
    case IFLA_STATS:
    {
      struct rtnl_link_stats *st = (struct rtnl_link_stats*)(rta + 1);
      snprintf(str, len, "%s \n"
        "\trx_packets:%u tx_packets:%u rx_bytes:%u tx_bytes:%u rx_errors:%u \n"
        "\ttx_errors:%u rx_dropped:%u tx_dropped:%u multicast:%u collisions:%u \n"
        "\trx_length_errors:%u rx_over_errors:%u rx_crc_errors:%u rx_frame_errors:%u \n"
        "\trx_fifo_errors:%u rx_missed_errors:%u tx_aborted_errors:%u \n"
        "\ttx_carrier_errors:%u tx_fifo_errors:%u tx_heartbeat_errors:%u \n"
        "\ttx_window_errors:%u rx_compressed:%u tx_compressed:%u",
        rta_type_to_str(rta->rta_type),
        st->rx_packets, st->tx_packets, st->rx_bytes, st->tx_bytes, st->rx_errors,
        st->tx_errors, st->rx_dropped, st->tx_dropped, st->multicast, st->collisions,
        st->rx_length_errors, st->rx_over_errors, st->rx_crc_errors, st->rx_frame_errors,
        st->rx_fifo_errors, st->rx_missed_errors, st->tx_aborted_errors, st->tx_carrier_errors,
        st->tx_fifo_errors, st->tx_heartbeat_errors, st->tx_window_errors, st->rx_compressed,
        st->tx_compressed);
      return str;
    }
    case IFLA_MAP:
    {
      struct rtnl_link_ifmap* ifmap = (struct rtnl_link_ifmap*)(rta + 1);
      snprintf(str, len, "%s \n"
        "\tmem_start:%llu mem_end:%llu \n"
        "\tbase_addr:%llu irq:%u dma:%u port:%u",
        rta_type_to_str(rta->rta_type),
        ifmap->mem_start, ifmap->mem_end, ifmap->base_addr,
        ifmap->irq, ifmap->dma, ifmap->port);
      return str;
    }
    case IFLA_PROMISCUITY:
    {
      uint32_t promisc = *(uint32_t*)(rta+1);
      snprintf(str, len, "%s <%s>",
          rta_type_to_str(rta->rta_type),
          promisc?"promisc=on":"promisc=off");
      return str;
    }

    case IFLA_PROTO_DOWN: // XXX ??? unknown..?
    case IFLA_OPERSTATE:
    case IFLA_LINKMODE:
    case IFLA_CARRIER:
    {
      uint8_t num = *(uint8_t*)(rta+1);
      snprintf(str, len, "%s <%u>",
          rta_type_to_str(rta->rta_type), num);
      return str;
    }
    case IFLA_NUM_TX_QUEUES:
    case IFLA_NUM_RX_QUEUES:
    case IFLA_MTU:
    case IFLA_LINK:
    case IFLA_GROUP:
    case IFLA_QDISC:
    case IFLA_CARRIER_CHANGES:
    case IFLA_TXQLEN:
    {
      uint32_t num = *(uint32_t*)(rta+1);
      snprintf(str, len, "%s <%u>",
          rta_type_to_str(rta->rta_type), num);
      return str;
    }
    case IFLA_LINKINFO:
    {
      uint8_t* ptr = (uint8_t*)(rta + 1);
      size_t len = rta->rta_len-sizeof(struct rtattr);
      carrydump(stdout, "IFLA_LINKINFO", ptr, len);
      snprintf(str, len, "IFLA_LINKINFO");
      return str;
    }
    default:
    {
#if 0
      snprintf(str, len, "%s unsupport-data=<...skipped>",
          rta_type_to_str(rta->rta_type));
#else
      char dstr[10000];
      memset(dstr, 0, sizeof(dstr));
      uint8_t* ptr = (uint8_t*)(rta + 1);
      for (size_t i=0; i<rta->rta_len-sizeof(struct rtattr); i++) {
        char sub_str[6];
        snprintf(sub_str, sizeof(sub_str), "%02x", ptr[i]);
        strncat(dstr, sub_str, strlen(sub_str));
      }
      snprintf(str, len, "%s unsupport-data=<%s>",
          rta_type_to_str(rta->rta_type), dstr);
      hexdump(stdout, ptr, rta->rta_len-sizeof(struct rtattr));
#endif
      return str;
    }
  }
}

#endif /* _NETLINK_RTATTR_H_ */
