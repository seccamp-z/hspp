/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>
#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <rte_bpf.h>
#include "bpf.h"

static uint8_t pkt[] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x11, 0x22,
  0x33, 0x44, 0x55, 0x66, 0x08, 0x06, 0x00, 0x01,
  0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
  0x00, 0x00,
};

int
main(int argc, char **argv)
{
  int ret = rte_eal_init(argc, argv);
  if (ret < 0) rte_panic("Cannot init EAL\n");

  const char *fname = "./code/t1.o";
  struct rte_bpf* bpf = mbuf_bpf_elf_load(fname);
  if (bpf == NULL) {
    printf("MISSED exit\n");
    return 1;
  }

  uint64_t bpfret = rte_bpf_exec(bpf, pkt);
  printf("bpfret:%lu 0x%lx\n", bpfret, bpfret);

  rte_bpf_dump(stdout, bpf);

  rte_bpf_destroy(bpf);
  return 0;
}

