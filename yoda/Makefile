
CXXFLAGS +=
LDFLAGS +=

SRC = main.cc
OBJ = $(SRC:.cc=.o)
TARGET = a.out

all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)

