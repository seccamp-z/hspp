
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_common.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include "dpdk.h"
#define unused(a) (void)(a)


int main(int argc, char **argv)
{
  int ret = rte_eal_init(argc, argv);
  if (ret < 0) rte_exit(EXIT_FAILURE, "Invalid EAL parameters\n");

  struct rte_mempool* mp = NULL;
  mp = rte_pktmbuf_pool_create("mp", 8192, 0, 0, RTE_MBUF_DEFAULT_BUF_SIZE, 0);
  if (!mp) rte_exit(EXIT_FAILURE, "Invalid MP parameters\n");

  struct rte_eth_conf port_conf;
  memset(&port_conf, 0, sizeof(struct rte_eth_conf));
  port_conf.rxmode.mq_mode = ETH_MQ_RX_NONE;
  port_conf.rxmode.max_rx_pkt_len = ETHER_MAX_LEN;
  port_conf.rxmode.header_split = 0;
  port_conf.rxmode.jumbo_frame = 1;
  port_conf.rxmode.enable_scatter = 1;
  port_conf.txmode.mq_mode = ETH_MQ_TX_NONE;
  rte_eth_dev_configure_easy(0,1,1,&port_conf,mp);

  uint8_t buf0[1000], buf1[1000], buf2[1000];
  memset(buf0, 0x11, sizeof(buf0));
  memset(buf1, 0x22, sizeof(buf1));
  memset(buf2, 0x33, sizeof(buf2));
  struct rte_mbuf* m0 = rte_pktmbuf_alloc_set(mp, buf0, sizeof(buf0));
  struct rte_mbuf* m1 = rte_pktmbuf_alloc_set(mp, buf1, sizeof(buf1));
  struct rte_mbuf* m2 = rte_pktmbuf_alloc_set(mp, buf2, sizeof(buf2));

  rte_pktmbuf_chain(m0, m1);
  rte_pktmbuf_chain(m0, m2);
  rte_pktmbuf_linearize(m0);
  rte_pktmbuf_dump(stdout, m0, rte_pktmbuf_pkt_len(m0));

  int ntx = rte_eth_tx_burst(0, 0, (struct rte_mbuf**)&m0, 1);
  if (ntx != 1) rte_exit(EXIT_FAILURE, "tx_burst\n");

  rte_pktmbuf_free(m0);
  rte_pktmbuf_free(m1);
  rte_mempool_free(mp);
  return 0;
}

