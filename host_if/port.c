
#include "port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_ring.h>
#include "tap.h"

#define UNUSED(a) (void)(a)

static void
port_configure(uint8_t port, size_t nb_rxq, size_t nb_txq,
      const struct rte_eth_conf* port_conf, struct rte_mempool* mp)
{
  const size_t rx_ring_size = 128;
  const size_t tx_ring_size = 512;

  int ret = rte_eth_dev_configure(port, nb_rxq, nb_txq, port_conf);
  if (ret < 0) rte_exit(EXIT_FAILURE, "rte_eth_dev_configure\n");

  for (size_t q=0; q<nb_rxq; q++) {
    int ret = rte_eth_rx_queue_setup(port, q, rx_ring_size,
        rte_eth_dev_socket_id(port), NULL, mp);
    if (ret < 0) rte_exit(EXIT_FAILURE, "rte_eth_rx_queue_setup\n");
  }

  for (size_t q=0; q<nb_txq; q++) {
    struct rte_eth_txconf txconf;
    memset(&txconf, 0x0, sizeof(txconf));
    txconf.txq_flags &= ~ETH_TXQ_FLAGS_NOMULTSEGS;
    int ret = rte_eth_tx_queue_setup(port, q, tx_ring_size,
        rte_eth_dev_socket_id(port), &txconf);
    if (ret < 0) rte_exit(EXIT_FAILURE, "rte_eth_tx_queue_setup\n");
  }

  ret = rte_eth_dev_start(port);
  if (ret < 0) rte_exit(EXIT_FAILURE, "rte_eth_dev_start\n");
  rte_eth_promiscuous_enable(port);
}

port_t*
port_alloc(uint16_t dpdk_pid, size_t nrxq, size_t ntxq,
            uint32_t ifaddr_le, uint32_t ifmask_le,
            const char* ifname)
{
  port_t* port = (port_t*)malloc(sizeof(port_t));
  if (!port) rte_exit(EXIT_FAILURE, "port_alloc: malloc miss\n");

  const size_t MBUF_CACHE_SIZE = 250;
  const size_t num_mbufs = 8191;
  char mpname[100];
  snprintf(mpname, sizeof(mpname), "PORT%u_MBUFPOOL", dpdk_pid);
  port->mp = rte_pktmbuf_pool_create(mpname, num_mbufs,
                MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE,
                rte_eth_dev_socket_id(dpdk_pid));
  if (port->mp == NULL) {
    rte_exit(EXIT_FAILURE, "Cannot create mbuf pool %s\n", mpname);
  }

  struct rte_eth_conf port_conf;
  memset(&port_conf, 0, sizeof(struct rte_eth_conf));
  port_conf.rxmode.mq_mode = ETH_MQ_RX_NONE;
  port_conf.rxmode.max_rx_pkt_len = ETHER_MAX_LEN;
  port_conf.rxmode.header_split = 0;
  port_conf.rxmode.jumbo_frame = 1;
  port_conf.rxmode.enable_scatter = 1;
  port_conf.rxmode.ignore_offload_bitfield = 1;

  port->dpdk_pid = dpdk_pid;
  port->nrxq = nrxq;
  port->ntxq = ntxq;
  port_configure(port->dpdk_pid,
      port->nrxq, port->ntxq,
      &port_conf, port->mp);
  rte_eth_macaddr_get(port->dpdk_pid, &port->hwaddr);
  printf("%s addr: %02x:%02x:%02x:%02x:%02x:%02x \n", ifname,
      port->hwaddr.addr_bytes[0], port->hwaddr.addr_bytes[1],
      port->hwaddr.addr_bytes[2], port->hwaddr.addr_bytes[3],
      port->hwaddr.addr_bytes[4], port->hwaddr.addr_bytes[5]);

  port->hostif_fd = tap_alloc(ifname, ifaddr_le, ifmask_le, &port->hwaddr);
  uint32_t ringflags = 0;
  size_t ringsize = 8192;
  char ringname[100];
  snprintf(ringname, sizeof(ringname), "PORT%u_HOSTRING", dpdk_pid);
  port->from_hostif = rte_ring_create(ringname, ringsize, 0, ringflags);
  if (port->from_hostif == NULL) rte_exit(EXIT_FAILURE, "Cannot create ring\n");

  return port;
}

void
port_free(port_t* port)
{
  tap_free(port->hostif_fd);
  rte_eth_dev_stop(port->dpdk_pid);
  rte_mempool_free(port->mp);
  rte_ring_free(port->from_hostif);
}

