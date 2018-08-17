
#ifndef TAP_H
#define TAP_H

#include <stdint.h>
struct rte_mempool;
struct rte_mbuf;

struct rte_mbuf* tap_read(int fd, struct rte_mempool* mp);
void tap_send(int fd, struct rte_mbuf* m);
int tap_alloc(uint32_t addr_little);

#endif /* TAP_H */

