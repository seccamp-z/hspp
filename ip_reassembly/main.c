
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <rte_common.h>
#include <rte_eal.h>
#include <rte_cycles.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_ip_frag.h>
#include "pcapw.h"

struct rte_mempool* mp = NULL;
struct rte_ip_frag_tbl *tbl = NULL;
struct rte_ip_frag_death_row dr;

static void recvpkt(pcap_t* ph)
{
  const uint8_t* ptr;
  size_t len = pcapw_recv(ph, &ptr);
  struct rte_mbuf* m = rte_pktmbuf_alloc(mp);
  rte_memcpy(rte_pktmbuf_mtod(m, uint8_t*), ptr, len);
  m->pkt_len = len;
  m->data_len = len;

  struct ether_hdr *eth_hdr = rte_pktmbuf_mtod(m, struct ether_hdr *);
  if (eth_hdr->ether_type == htons(0x0800)) {
    struct ipv4_hdr *ip_hdr = (struct ipv4_hdr *)(eth_hdr + 1);
    if (rte_ipv4_frag_pkt_is_fragmented(ip_hdr)) {

      m->l2_len = sizeof(*eth_hdr);
      m->l3_len = sizeof(*ip_hdr);
      uint64_t tms = rte_get_tsc_cycles();
      struct rte_mbuf* mo = rte_ipv4_frag_reassemble_packet(tbl, &dr, m, tms, ip_hdr);
      if (mo == NULL) {
        return;
      }

      printf("reassembled\n");
      rte_pktmbuf_dump(stdout, mo, 0);
      if (mo != m) {
        m = mo;
        eth_hdr = rte_pktmbuf_mtod(m, struct ether_hdr *);
        ip_hdr = (struct ipv4_hdr *)(eth_hdr + 1);
      }

    }
  }
}

int main(int argc, char **argv)
{
  int ret = rte_eal_init(argc, argv);
  if (ret < 0) rte_exit(EXIT_FAILURE, "Invalid EAL parameters\n");

  mp = rte_pktmbuf_pool_create("mp", 8192, 0, 0, RTE_MBUF_DEFAULT_BUF_SIZE, 0);
  if (!mp) rte_exit(EXIT_FAILURE, "Invalid MP parameters\n");

  const uint32_t max_flow_num = 0x1000;
  const uint32_t max_flow_ttl = (3600 * MS_PER_S);
  const uint32_t bucket_entries = 16;
  const uint64_t frag_cycles = (rte_get_tsc_hz() + MS_PER_S - 1) / MS_PER_S * max_flow_ttl;
  tbl = rte_ip_frag_table_create(max_flow_num,
          bucket_entries, max_flow_num, frag_cycles, 0);
  if (!tbl) rte_exit(EXIT_FAILURE, "rte_ip_frag_table_create\n");

  pcap_t* ph = pcapw_open_offline("in.pcap");
  recvpkt(ph);
  recvpkt(ph);
  recvpkt(ph);
  recvpkt(ph);
  recvpkt(ph);
  recvpkt(ph);
  recvpkt(ph);

  rte_ip_frag_table_destroy(tbl);
  rte_mempool_free(mp);
  pcapw_close(ph);
  return 0;
}

