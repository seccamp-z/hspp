/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation
 */

#include <stdint.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>

uint64_t
entry(void *pkt)
{
	struct ether_header *eh = (void *)pkt;
	if (eh->ether_type != htons(0x0806))
		return 0;

	struct ether_arp *ah = (struct ether_arp*)(eh + 1);
  if (ah->ea_hdr.ar_op == htons(1)) return 1;
  else return 0;
	/* if (iphdr->protocol != 17 || (iphdr->frag_off & 0x1ffff) != 0 || */
	/* 		iphdr->daddr != htonl(0x1020304)) */
	/* 	return 0; */
  /*  */
	/* int hlen = iphdr->ihl * 4; */
	/* struct udphdr *udphdr = (void *)iphdr + hlen; */
  /*  */
	/* if (udphdr->dest != htons(5000)) */
	/* 	return 0; */
  /*  */
	/* return 1; */
}
