
RTE_SDK := /home/slankdev/dpdk.v18.11

CC = clang
CFLAGS += -U __GNUC__ -O0 -g
CFLAGS += -I$(RTE_SDK)/$(RTE_TARGET)/include/
CFLAGS += -Wno-int-to-void-pointer-cast
CFLAGS += -target bpf

SRC = t1.c t2.c t3.c
OBJ = $(SRC:.c=.o)

all: $(OBJ)
clean:
	rm -rf $(OBJ)

