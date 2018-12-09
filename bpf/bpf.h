
#include <rte_bpf.h>

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
