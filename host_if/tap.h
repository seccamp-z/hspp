
#ifndef TAP_H
#define TAP_H

#include <stdint.h>
struct rte_mempool;
struct rte_mbuf;
struct ether_addr;

struct rte_mbuf* tap_recv(int fd, struct rte_mempool* mp);
void tap_send(int fd, struct rte_mbuf* m);
int tap_alloc(const char* ifname,
      uint32_t addr_le, uint32_t mask_le,
      struct ether_addr* hwaddr);
void tap_free(int fd);

#endif /* TAP_H */

