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
#include <rte_ring.h>
#include "tap.h"

#define UNUSED(a) (void)(a)
#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024
#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

int hostif_fd = -1;
const uint32_t hostif_addr = 0x0a000001;
struct rte_mempool* mp = NULL;
struct rte_ring* from_hostif = NULL;
struct ether_addr ifhwaddr;

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

static void* fwd_main(void* dum)
{
  UNUSED(dum);
  const uint16_t nb_ports = rte_eth_dev_count();
  for (uint16_t port = 0; port < nb_ports; port++)
    if (rte_eth_dev_socket_id(port) > 0 &&
        rte_eth_dev_socket_id(port) !=
            (int)rte_socket_id())
      printf("WARNING, port %u is on remote NUMA node to "
          "polling thread.\n\tPerformance will "
          "not be optimal.\n", port);

  printf("Core %u forwarding main\n", rte_lcore_id());
  for (;;) {

    /*
     * Check Packet from hostif
     */
    if (rte_ring_empty(from_hostif) == 0) {
      struct rte_mbuf* mbufs[BURST_SIZE];
      size_t ndeq = rte_ring_dequeue_burst(from_hostif, (void**)mbufs, BURST_SIZE, NULL);
      for (size_t i=0; i<ndeq; i++) {
        printf("receive from host\n");
        uint16_t ntx = rte_eth_tx_burst(0, 0, &mbufs[i], 1);
        if (ntx != 1) rte_pktmbuf_free(mbufs[i]);
      }
    }

    /*
     * Receive Physical Interface
     */
    const uint16_t port = 0;
    struct rte_mbuf *bufs[BURST_SIZE];
    const uint16_t nb_rx = rte_eth_rx_burst(port, 0, bufs, BURST_SIZE);
    if (unlikely(nb_rx == 0)) continue;
    for (uint16_t i=0; i<nb_rx; i++) {
      port_input(bufs[i]);
    }

  } /* for (;;) */
  return NULL;
}

static void* tap_main(void* dum)
{
  UNUSED(dum);
  printf("Core %u tap main\n", rte_lcore_id());
  while (1) {
    struct rte_mbuf* m = tap_recv(hostif_fd, mp);
    rte_ring_enqueue(from_hostif, m);
  }
  return NULL;
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

  struct rte_eth_conf port_conf;
  memset(&port_conf, 0, sizeof(struct rte_eth_conf));
  port_conf.rxmode.mq_mode = ETH_MQ_RX_NONE;
  port_conf.rxmode.max_rx_pkt_len = ETHER_MAX_LEN;
  port_conf.rxmode.header_split = 0;
  port_conf.rxmode.jumbo_frame = 1;
  port_conf.rxmode.enable_scatter = 1;
  port_conf.rxmode.ignore_offload_bitfield = 1;
  port_configure(0,1,1,&port_conf,mp);
  rte_eth_macaddr_get(0, &ifhwaddr);
  printf("port0 addr: %02x:%02x:%02x:%02x:%02x:%02x \n",
      ifhwaddr.addr_bytes[0], ifhwaddr.addr_bytes[1], ifhwaddr.addr_bytes[2],
      ifhwaddr.addr_bytes[3], ifhwaddr.addr_bytes[4], ifhwaddr.addr_bytes[5]);

  hostif_fd = tap_alloc(hostif_addr, &ifhwaddr);
  uint32_t ringflags = 0;
  size_t ringsize = 8192;
  from_hostif = rte_ring_create("RING_FROM_HOSTIF", ringsize, 0, ringflags);
  if (from_hostif == NULL) rte_exit(EXIT_FAILURE, "Cannot create ring\n");

  pthread_t fwd_thread;
  pthread_t tap_thread;
  ret = pthread_create(&fwd_thread, NULL, fwd_main, NULL);
  if (ret != 0) rte_exit(EXIT_FAILURE, "Failed create fwd_thread\n");
  ret = pthread_create(&tap_thread, NULL, tap_main, NULL);
  if (ret != 0) rte_exit(EXIT_FAILURE, "Failed create tap_thread\n");
  pthread_join(fwd_thread, NULL);
  pthread_join(tap_thread, NULL);
  return 0;
}

