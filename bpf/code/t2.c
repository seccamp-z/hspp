/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation
 */
#include <stdint.h>
#include <net/ethernet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include "mbuf.h"

#define bswap16(v) \
 ((((uint16_t)(v) & UINT16_C(0x00ff)) << 8) | \
  (((uint16_t)(v) & UINT16_C(0xff00)) >> 8));

#define bswap32(v) \
 ((((uint32_t)(v) & UINT32_C(0x000000ff)) << 24) | \
  (((uint32_t)(v) & UINT32_C(0x0000ff00)) <<  8) | \
  (((uint32_t)(v) & UINT32_C(0x00ff0000)) >>  8) | \
  (((uint32_t)(v) & UINT32_C(0xff000000)) >> 24));

/*
 * MODIFY IP SRC FIELD
 */
uint64_t
eentry(void *pkt)
{
  struct rte_mbuf* m = (void*)pkt;
  struct ether_header *eh = rte_pktmbuf_mtod(m, void*);
  uint16_t type = bswap16(eh->ether_type);
  if (type != 0x0800)
    return 0;

  struct iphdr* ih = (void*)(eh + 1);
  uint32_t saddr = bswap32(ih->saddr);
  if (saddr == 0x0a000002)
    ih->saddr = bswap32(0x0a000014);
  return 0;
}
