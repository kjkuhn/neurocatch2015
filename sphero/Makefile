CC = g++
CFLAGS = -std=c++11 -Wall -c
LDFLAGS = -lbluetooth -lsphero
SRC = test.cpp
OBJ = $(SRC:%.cpp=%.o)
EXEC = sphero

all: $(SRC) $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS) 
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@

clean:
	rm -rf *.o $(EXEC)

