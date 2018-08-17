/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2015 Intel Corporation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <rte_eal.h>
#include <rte_lcore.h>
#include "tap.h"
#include "port.h"

#define UNUSED(a) (void)(a)
#define BURST_SIZE 32
port_t* port[2];

static void port_input(struct rte_mbuf* m, port_t* pp)
{
  struct ether_hdr* eh = rte_pktmbuf_mtod(m, struct ether_hdr*);
  switch (htons(eh->ether_type)) {
    case 0x86dd:
    case 0x0800:
    case 0x0806:
      tap_send(pp->hostif_fd, m);
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
  printf("Core %u forwarding main\n", rte_lcore_id());
  const size_t n_ports = rte_eth_dev_count();
  while (true) {
    for (size_t pid=0; pid<n_ports; pid++) {

      /*
       * Check Packet from hostif
       */
      if (rte_ring_empty(port[pid]->from_hostif) == 0) {
        struct rte_mbuf* mbufs[BURST_SIZE];
        size_t ndeq = rte_ring_dequeue_burst(port[pid]->from_hostif,
                          (void**)mbufs, BURST_SIZE, NULL);
        for (size_t i=0; i<ndeq; i++) {
          printf("receive from host\n");
          uint16_t ntx = rte_eth_tx_burst(port[pid]->dpdk_pid, 0, &mbufs[i], 1);
          if (ntx != 1) rte_pktmbuf_free(mbufs[i]);
        }
      }

      /*
       * Receive Physical Interface
       */
      struct rte_mbuf *bufs[BURST_SIZE];
      const uint16_t nb_rx = rte_eth_rx_burst(port[pid]->dpdk_pid, 0, bufs, BURST_SIZE);
      if (unlikely(nb_rx == 0)) continue;
      for (uint16_t i=0; i<nb_rx; i++) {
        port_input(bufs[i], port[pid]);
      }

    } /* for (size_t pid=0; pid<n_ports; pid++) */
  } /* while (true) */
  return NULL;
}

static void* tap_main(void* dum)
{
  UNUSED(dum);
  printf("Core %u tap main\n", rte_lcore_id());
  const size_t n_ports = rte_eth_dev_count();
  while (1) {
    for (size_t pid=0; pid<n_ports; pid++) {
      struct rte_mbuf* m = tap_recv(port[pid]->hostif_fd, port[pid]->mp);
      rte_ring_enqueue(port[pid]->from_hostif, m);
    }
  }
  return NULL;
}

int main(int argc, char *argv[])
{
  int ret = rte_eal_init(argc, argv);
  if (ret < 0) rte_exit(EXIT_FAILURE, "rte_eal_init\n");

  port[0] = port_alloc(0, 1, 1, 0x0a000001, 0xffffff00, "ge-0-0-0");
  port[1] = port_alloc(1, 1, 1, 0x0a010001, 0xffffff00, "ge-0-0-1");

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

