
CFLAGS +=
LDFLAGS +=

SRC = main.c
OBJ = $(SRC:.c=.o)
TARGET = a.out

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)

ifreset:
	sudo ip addr flush dev lo
	sudo ip addr add 127.0.0.1/8 dev lo
