
#ifndef PORT_H
#define PORT_H

#include <stdint.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
struct rte_mempool;
struct rte_ring;

typedef struct port {
  int hostif_fd;
  uint16_t dpdk_pid;
  size_t nrxq;
  size_t ntxq;
  uint32_t hostif_addr;
  struct ether_addr hwaddr;
  struct rte_ring* from_hostif;
  struct rte_mempool* mp;
} port_t;

port_t* port_alloc(uint16_t dpdk_pid,
            size_t nrxq, size_t ntxq,
            uint32_t ifaddr_le,
            uint32_t ifmask_le);
void port_free(port_t* port);

#endif /* PORT_H */

