
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "netlink_socket.h"
#include "netlink_dump.h"

int main(int argc, char** argv)
{
  int sock = netlink_sock_open();
  while (true) {
    uint8_t buf[1000];
    ssize_t ret = read(sock, buf, sizeof(buf));
    if (ret < 0) {
      return -1;
    }

    for (struct nlmsghdr* hdr = (struct nlmsghdr*)buf;
         NLMSG_OK(hdr, ret); hdr = NLMSG_NEXT(hdr, ret)) {
      netlink_msg_dump(stdout, hdr);
      printf("\n\n\n");
    }

  } /* while (true) */
  netlink_sock_close(sock);
}

