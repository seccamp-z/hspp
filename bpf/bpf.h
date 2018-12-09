
#ifndef _BPF_H_
#define _BPF_H_

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
#include <rte_byteorder.h>
#define XDP_DROP 1

/* struct rte_bpf in dpdk source */
struct bpf {
  struct rte_bpf_prm prm;
  struct rte_bpf_jit jit;
  size_t sz;
  uint32_t stack_sz;
};

static inline void
rte_bpf_dump(FILE* f, const struct rte_bpf* bpf_)
{
  const struct bpf* bpf = (const void*)bpf_;

  fprintf(f, "bpf <%s>@%p\n", "/pmd/md/t.o", bpf);
  const uint64_t* ins = (const uint64_t*)(bpf->prm.ins);
  for (size_t i=0; i<bpf->prm.nb_ins; i++) {
    printf("  0x%04lx: %016lx\n", i*8, rte_bswap64(ins[i]));
  }
}

static inline struct rte_bpf*
mbuf_bpf_elf_load (const char* fname)
{
  static const struct rte_bpf_xsym bpf_xsym[] = {
    {
      .name = RTE_STR(stdout),
      .type = RTE_BPF_XTYPE_VAR,
      .var = {
        .val = &stdout,
        .desc = {
          .type = RTE_BPF_ARG_PTR,
          .size = sizeof (stdout),
        },
      },
    },
    {
      .name = RTE_STR(rte_pktmbuf_dump),
      .type = RTE_BPF_XTYPE_FUNC,
      .func = {
        .val = (void *) rte_pktmbuf_dump,
        .nb_args = 3,
        .args = {
          [0] = {
            .type = RTE_BPF_ARG_RAW,
            .size = sizeof (uintptr_t),
          },
          [1] = {
            .type = RTE_BPF_ARG_PTR_MBUF,
            .size = sizeof (struct rte_mbuf),
          },
          [2] = {
            .type = RTE_BPF_ARG_RAW,
            .size = sizeof (uint32_t),
          },
        },
      },
    },
  };

	struct rte_bpf_prm prm;
	memset (&prm, 0, sizeof (prm));
	prm.xsym = bpf_xsym;
	prm.nb_xsym = RTE_DIM(bpf_xsym);
  prm.prog_arg.type = RTE_BPF_ARG_PTR_MBUF;
  prm.prog_arg.size = sizeof(struct rte_mbuf);
  prm.prog_arg.buf_size = RTE_MBUF_DEFAULT_BUF_SIZE;
	const char *sname = ".text";
  struct rte_bpf* bpf = rte_bpf_elf_load (&prm, fname, sname);
  if (bpf == NULL)
    {
      fprintf (stderr, "%s:%d: in function %s()\n",
          __FILE__, __LINE__, __func__);
      return NULL;
    }
  return bpf;
}

#endif /* _BPF_H_ */
