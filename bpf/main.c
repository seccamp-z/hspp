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

static const struct rte_bpf_xsym bpf_xsym[] = {
	{
		.name = RTE_STR(stdout),
		.type = RTE_BPF_XTYPE_VAR,
		.var = {
			.val = &stdout,
			.desc = {
				.type = RTE_BPF_ARG_PTR,
				.size = sizeof(stdout),
			},
		},
	},
	{
		.name = RTE_STR(rte_pktmbuf_dump),
		.type = RTE_BPF_XTYPE_FUNC,
		.func = {
			.val = (void *)rte_pktmbuf_dump,
			.nb_args = 3,
			.args = {
				[0] = {
					.type = RTE_BPF_ARG_RAW,
					.size = sizeof(uintptr_t),
				},
				[1] = {
					.type = RTE_BPF_ARG_PTR_MBUF,
					.size = sizeof(struct rte_mbuf),
				},
				[2] = {
					.type = RTE_BPF_ARG_RAW,
					.size = sizeof(uint32_t),
				},
			},
		},
	},
};

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
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

	struct rte_bpf_prm prm;
	memset(&prm, 0, sizeof(prm));
	prm.xsym = bpf_xsym;
	prm.nb_xsym = RTE_DIM(bpf_xsym);
  prm.prog_arg.type = RTE_BPF_ARG_PTR_MBUF;
  prm.prog_arg.size = sizeof(struct rte_mbuf);
  prm.prog_arg.buf_size = RTE_MBUF_DEFAULT_BUF_SIZE;
  const char *fname = "./code/t1.o";
	const char *sname = ".text";

  struct rte_bpf* bpf = rte_bpf_elf_load(&prm, fname, sname);
  if (bpf == NULL) {
    printf("MISSED exit\n");
    return 1;
  }
  printf("bpf:%p\n", bpf);

  uint64_t bpfret = rte_bpf_exec(bpf, pkt);
  printf("bpfret:%lu 0x%lx\n", bpfret, bpfret);
	return 0;
}
