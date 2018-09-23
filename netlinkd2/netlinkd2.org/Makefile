
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

