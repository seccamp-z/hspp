
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <pcap.h>

static inline pcap_t* pcapw_open_offline(const char* filepath)
{
  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_t* handle = pcap_open_offline(filepath, errbuf);
  if (!handle) {
    fprintf(stderr, "pcap_open_offline: %s", errbuf);
    exit(1);
  }
  return handle;
}

static inline size_t pcapw_recv(pcap_t* handle, const uint8_t** bufptr)
{
  struct pcap_pkthdr header;
  *bufptr = pcap_next(handle, &header);
  return header.len;
}

static inline void pcapw_close(pcap_t* handle)
{
  pcap_close(handle);
}

