
RTE_SDK = $(HOME)/dpdk.v18.11

CC = clang
CFLAGS = -O2 -U __GNUC__
CFLAGS = -I$(RTE_SDK)/$(RTE_TARGET)/include
CFLAGS = -Wno-int-to-void-pointer-cast
CFLAGS = -target bpf

SRC = t1.c t2.c
OBJ = $(SRC:.c=.o)

all: $(OBJ)
clean:
	rm -rf $(OBJ)

