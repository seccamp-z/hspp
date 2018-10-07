
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


