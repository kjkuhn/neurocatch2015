CC = g++
CFLAGS = -Wall -c -std=c++11 
LDFLAGS = -L /usr/lib/x86_64-linux-gnu/ -lpthread -lboost_thread -lboost_system
INCLUDE = -I /usr/include/ -I /usr/include/boost/ 
SRC = edvs_pos.cpp
OBJ = $(SRC:%.cpp=%.o)
EXEC = edvs_pos

all: $(SRC) $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS) 
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@

clean:
	rm -rf $(EXEC) $(OBJ)

test:
	$(CC) -std=c++11 $(SRC) -o $(EXEC) $(INCLUDE) $(LDFLAGS)
