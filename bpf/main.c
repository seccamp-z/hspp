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

static int
lcore_hello(__attribute__((unused)) void *arg)
{
	unsigned lcore_id;
	lcore_id = rte_lcore_id();
	printf("hello from core %u\n", lcore_id);
	return 0;
}

int
main(int argc, char **argv)
{
	int ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

  struct rte_bpf_prm prm;
  const char* fname = "bytecode/t1.o";
  const char* sname = "function";
  struct rte_bpf* bpf = rte_bpf_elf_load(&prm, fname, sname);
  if (bpf == NULL)
    rte_panic("rte_bpf_elf_load");

	lcore_hello(NULL);
	rte_eal_mp_wait_lcore();
	return 0;
}
