
CFLAGS += -I../
# LDFLAGS += -L../

all:
	$(CC) $(CFLAGS) analyze_rta_IFLA_LINKINFO.c -o analyze_rta_IFLA_LINKINFO.out $(LDFLAGS)
	$(CC) $(CFLAGS) analyze_rta_IFLA_AF_SPEC.c -o analyze_rta_IFLA_AF_SPEC.out $(LDFLAGS)
	cp analyze_rta_IFLA_AF_SPEC.out a.out
