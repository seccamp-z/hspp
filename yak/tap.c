
#include "tap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>

#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_ether.h>

struct rte_mbuf* tap_recv(int fd, struct rte_mempool* mp)
{
  struct rte_mbuf* m = rte_pktmbuf_alloc(mp);
  ssize_t len = read(fd, rte_pktmbuf_mtod(m, void*),
                      m->buf_len-rte_pktmbuf_headroom(m));
  if (len < 0) rte_exit(EXIT_FAILURE, "tap receive failed\n");
  m->pkt_len = m->data_len = len;
  return m;
}

void tap_send(int fd, struct rte_mbuf* m)
{
  ssize_t len = write(fd, rte_pktmbuf_mtod(m, void*), m->pkt_len);
  if (len < 0) rte_exit(EXIT_FAILURE, "tap send failed\n");
  rte_pktmbuf_free(m);
}

void tap_free(int fd)
{
  close(fd);
}

int tap_alloc(const char* ifname,
              struct ether_addr* hwaddr)
{
  /* open tap interface */
  int fd = open("/dev/net/tun", O_RDWR);
  if (fd < 0) rte_exit(EXIT_FAILURE, "Cannot create tap\n");
  struct ifreq ifr;
  memset(&ifr, 0x0, sizeof(ifr));
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  int ret = ioctl(fd, TUNSETIFF, (void*)&ifr);
  if (ret < 0) rte_exit(EXIT_FAILURE, "ioctl TUNSETIFF failed\n");

  /* set hardware addr */
  ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
  ether_addr_copy(hwaddr, (struct ether_addr*)&ifr.ifr_hwaddr.sa_data);
  ret = ioctl(fd, SIOCSIFHWADDR, &ifr);
  if (ret < 0) rte_exit(EXIT_FAILURE, "ioctl SIOCSIFHWADDR failed\n");

  /* set link up */
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  memset(&ifr, 0x0, sizeof(ifr));
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
  ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
  if (ret < 0) rte_exit(EXIT_FAILURE, "ioctl SIOCGIFFLAGS failed\n");
  ifr.ifr_flags |= IFF_UP;
  ret = ioctl(sock, SIOCSIFFLAGS, &ifr);
  if (ret < 0) rte_exit(EXIT_FAILURE, "ioctl SIOCSIFFLAGS failed\n");
  close(sock);

  return fd;
}

