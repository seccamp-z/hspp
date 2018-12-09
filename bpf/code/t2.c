/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation
 */
#include <stdint.h>
#include <net/ethernet.h>

uint64_t
eentry(void *pkt)
{
  struct ether_header *ether_header = (void *)pkt;
	if (ether_header->ether_type == 0xdd86)
		return 0;
  else
    return 1;
}
