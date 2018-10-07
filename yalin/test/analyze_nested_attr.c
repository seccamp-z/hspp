
#include <yalin/yalin.h>

// IFLA_LINKINFO attr (vlan interface)
static uint8_t arry[] = {
  0x09, 0x00, 0x01, 0x00, 0x76, 0x6c, 0x61, 0x6e,    /*....vlan*/
  0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x02, 0x00,    /*.... ...*/
  0x06, 0x00, 0x05, 0x00, 0x81, 0x00, 0x00, 0x00,    /*........*/
  0x06, 0x00, 0x01, 0x00, 0x0a, 0x00, 0x00, 0x00,    /*........*/
  0x0c, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00,    /*........*/
  0xff, 0xff, 0xff, 0xff,                            /*....*/
};

uint16_t rtattr_type(const struct rtattr* rta)
{ return rta->rta_type; }

size_t rtattr_len(const struct rtattr* rta)
{ return rta->rta_len; }

size_t rtattr_payload_len(const struct rtattr* rta)
{ return rta->rta_len-sizeof(struct rtattr); }

uint8_t* rtattr_payload_ptr(const struct rtattr* rta)
{ return (uint8_t*)(rta + 1); }

size_t align(size_t blocklen, size_t alignlen)
{
  /* printf("blocklen: %zd \n", blocklen); */
  /* printf("alignlen: %zd \n", alignlen); */
  return (blocklen + alignlen - 1) & ~(alignlen - 1);
}

struct rtattr* rtattr_next(struct rtattr* rta, ssize_t* buflen)
{
  size_t seek_len = align(rtattr_len(rta), 4);
  if (*buflen <= 0) return NULL;
  if (seek_len <= 0) return NULL;

  if ((*buflen - seek_len) > sizeof(struct rtattr)) {
    struct rtattr* next_rta = (struct rtattr*)(((uint8_t*)rta) + seek_len);
    /* printf("%s: buflen=%zd, state(len,type): cur(0x%lx,0x%x) --seek(%zd)--> nxt(0x%lx,0x%x)\n", */
    /*     __func__, *buflen, */
    /*     rtattr_len(rta), rtattr_type(rta), seek_len, */
    /*     rtattr_len(next_rta), rtattr_type(next_rta)); */
    *buflen -= seek_len;
    return next_rta;
  } else {
    return NULL;
  }
}

uint8_t rtattr_read_8bit (const struct rtattr* attr)
{
  if (rtattr_payload_len(attr) > sizeof(uint8_t)) {
    fprintf(stderr, "%s: read miss (l,t)=(%zd,%u)\n", __func__,
        rtattr_payload_len(attr), rtattr_type(attr));
    fprintf(stderr, " - payload_len: %zd\n", rtattr_payload_len(attr));
    exit(1);
  }
  uint8_t val = *(uint8_t*)rtattr_payload_ptr(attr);
  /* printf("%s: val=%u,0x%x\n", __func__, val, val); */
  return val;
}

uint16_t rtattr_read_16bit(const struct rtattr* attr)
{
  if (rtattr_payload_len(attr) > sizeof(uint16_t)) {
    fprintf(stderr, "%s: read miss (l,t)=(%zd,%u)\n", __func__,
        rtattr_payload_len(attr), rtattr_type(attr));
    fprintf(stderr, " - payload_len: %zd\n", rtattr_payload_len(attr));
    exit(1);
  }
  uint16_t val = *(uint16_t*)rtattr_payload_ptr(attr);
  /* printf("%s: val=%u,0x%x\n", __func__, val, val); */
  return val;
}

uint32_t rtattr_read_32bit(const struct rtattr* attr)
{
  if (rtattr_payload_len(attr) > sizeof(uint32_t)) {
    fprintf(stderr, "%s: read miss (l,t)=(%zd,%u)\n", __func__,
        rtattr_payload_len(attr), rtattr_type(attr));
    fprintf(stderr, " - payload_len: %zd\n", rtattr_payload_len(attr));
    exit(1);
  }
  uint32_t val = *(uint32_t*)rtattr_payload_ptr(attr);
  /* printf("%s: val=%u,0x%x\n", __func__, val, val); */
  return val;
}

uint64_t rtattr_read_64bit(const struct rtattr* attr)
{
  if (rtattr_payload_len(attr) > sizeof(uint64_t)) {
    fprintf(stderr, "%s: read miss (l,t)=(%zd,%u)\n", __func__,
        rtattr_payload_len(attr), rtattr_type(attr));
    fprintf(stderr, " - payload_len: %zd\n", rtattr_payload_len(attr));
    exit(1);
  }
  uint64_t val = *(uint64_t*)rtattr_payload_ptr(attr);
  /* printf("%s: val=%lu,0x%lx\n", __func__, val, val); */
  return val;
}

size_t rtattr_read_str(const struct rtattr* attr, char* str, size_t strbuflen)
{
  if (rtattr_payload_len(attr) > strbuflen) {
    fprintf(stderr, "%s: read miss (type=%u)\n", __func__, rtattr_type(attr));
    printf("payloadlen: %zd\n", rtattr_payload_len(attr));
    printf("strbuflen: %zd\n", strbuflen);
    exit(1);
  }
  snprintf(str, strbuflen, "%s", rtattr_payload_ptr(attr));
  return align(rtattr_payload_len(attr), 4);
}

struct interface_info_vlan {
  uint16_t proto;
  uint16_t id;
  uint32_t flags;
};

void parse_vlan_attrs(struct rtattr* rta,
    struct interface_info_vlan* vlaninfo)
{
  size_t buflen = rtattr_len(rta);

  if (rtattr_type(rta) != IFLA_INFO_KIND)
    goto err_format_inval;

  char str[100];
  size_t readlen = rtattr_read_str(rta, str, sizeof(str));
  if (strcmp(str, "vlan") != 0)
    goto err_format_inval;

  rta = rtattr_next(rta, &buflen);
  bool ret_cond0 = rta;
  bool ret_cond1 = rtattr_type(rta) == IFLA_INFO_DATA;
  if (ret_cond0 && ret_cond1) {
    struct rtattr* nrta = (struct rtattr*)rtattr_payload_ptr(rta);
    ssize_t nbuflen = rtattr_payload_len(rta);

    while (nrta) {

      uint16_t type = rtattr_type(nrta);
      switch (type) {
        case IFLA_VLAN_ID:
          vlaninfo->id = rtattr_read_16bit(nrta);
          break;
        case IFLA_VLAN_FLAGS:
          vlaninfo->flags = rtattr_read_64bit(nrta);
          break;
        case IFLA_VLAN_PROTOCOL:
          vlaninfo->proto = rtattr_read_16bit(nrta);
          break;
        case IFLA_VLAN_UNSPEC:
        case IFLA_VLAN_EGRESS_QOS:
        case IFLA_VLAN_INGRESS_QOS:
          printf("unsupport ... skip type=0x%04x @%p\n", type, nrta);
          break;
      }

      nrta = rtattr_next(nrta, &nbuflen);
      if (!nrta || nbuflen<=0) break;
    }
    return;
  }

err_format_inval:
  fprintf(stderr, "%s: invalid format\n", __func__);
  exit(1);
}

int main()
{
  hexdump(stdout, arry, sizeof(arry));
  struct rtattr* attr = (struct rtattr*)arry;
  struct interface_info_vlan vi = {0};
  parse_vlan_attrs(attr, &vi);
  printf("vlan_infos");
  printf(", id:%u", vi.id);
  printf(", proto:0x%04x", vi.proto);
  printf(", flags:0x%04x", vi.flags);
  printf("\n");
}


