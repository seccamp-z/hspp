
CFLAGS += -Wall -Werror
CFLAGS += -fsanitize=address
LDFLAGS +=

SRC = main.c
OBJ = $(SRC:.c=.o)
TARGET = a.out

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)

re: clean all
