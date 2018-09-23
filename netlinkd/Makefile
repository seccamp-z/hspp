
IPROUTE2 = $(HOME)/git/iproute2
CFLAGS += -I $(IPROUTE2)/include
CFLAGS += -I../netlinkd/ -g -O0
LDFLAGS += -L $(IPROUTE2)/lib
LDFLAGS += -lnetlink

SRC = main.c
OBJ = $(SRC:.c=.o)
TARGET = a.out

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)

reset:
	sudo ip addr flush dev lo
	sudo ip addr add 127.0.0.1/8 dev lo

set:
	sudo ip addr add 1.1.1.1/32 dev lo

vlan-add:
	sudo ip link add link enp0s3 name enp0s3.10 type vlan id 10

vlan-del:
	sudo ip link del enp0s3.10

