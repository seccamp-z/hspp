/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2015 Intel Corporation
 */

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
#include "tap.h"

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024
#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

int hostif_fd = -1;
const uint32_t hostif_addr = 0x0a000001;
struct rte_mempool* mp = NULL;

static void port_configure(uint8_t port, size_t nb_rxq, size_t nb_txq,
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

static void port_input(struct rte_mbuf* m)
{
  struct ether_hdr* eh = rte_pktmbuf_mtod(m, struct ether_hdr*);
  switch (htons(eh->ether_type)) {
    case 0x0800:
    case 0x0806:
      tap_send(hostif_fd, m);
      break;
    default:
      printf("unknown ether_type 0x%04x\n", htons(eh->ether_type));
      rte_pktmbuf_free(m);
      break;
  }
  /* const uint16_t nb_tx = rte_eth_tx_burst(port ^ 1, 0, &bufs[i], 1); */
  /* if (unlikely(nb_tx != 1)) rte_pktmbuf_free(bufs[i]); */
}

static void lcore_main(void)
{
  const uint16_t nb_ports = rte_eth_dev_count();
  for (uint16_t port = 0; port < nb_ports; port++)
    if (rte_eth_dev_socket_id(port) > 0 &&
        rte_eth_dev_socket_id(port) !=
            (int)rte_socket_id())
      printf("WARNING, port %u is on remote NUMA node to "
          "polling thread.\n\tPerformance will "
          "not be optimal.\n", port);

  printf("\nCore %u forwarding packets. [Ctrl+C to quit]\n", rte_lcore_id());
  for (;;) {
    for (uint16_t port = 0; port < nb_ports; port++) {

      struct rte_mbuf *bufs[BURST_SIZE];
      const uint16_t nb_rx = rte_eth_rx_burst(port, 0, bufs, BURST_SIZE);
      if (unlikely(nb_rx == 0)) continue;

      for (uint16_t i=0; i<nb_rx; i++) {
        port_input(bufs[i]);
      }
    }
  }
}

int main(int argc, char *argv[])
{
  int ret = rte_eal_init(argc, argv);
  if (ret < 0) rte_exit(EXIT_FAILURE, "rte_eal_init\n");

  const uint16_t nb_ports = rte_eth_dev_count();
  if (nb_ports != 1) rte_exit(EXIT_FAILURE, "Error: #ports isn't 1\n");

  mp = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
    MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
  if (mp == NULL) rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

  hostif_fd = tap_alloc(hostif_addr);

  struct rte_eth_conf port_conf;
  memset(&port_conf, 0, sizeof(struct rte_eth_conf));
  port_conf.rxmode.mq_mode = ETH_MQ_RX_NONE;
  port_conf.rxmode.max_rx_pkt_len = ETHER_MAX_LEN;
  port_conf.rxmode.header_split = 0;
  port_conf.rxmode.jumbo_frame = 1;
  port_conf.rxmode.enable_scatter = 1;
  port_conf.rxmode.ignore_offload_bitfield = 1;
  port_configure(0,1,1,&port_conf,mp);

  lcore_main();
  return 0;
}
