CXX     = g++

CXXFLAGS        = -Wall -std=c++11 -pthread

CLIENT_OFILES  = ./client/client.o

SERVER_OFILES  = ./server/main.o ./server/connection.o ./server/game.o ./server/server.o

.SUFFIXES: .o .cpp

all: client server

client:	$(CLIENT_OFILES)
	$(CXX) $(CXXFLAGS) $(CLIENT_OFILES) -o myclient

server:	$(SERVER_OFILES)
	$(CXX) $(CXXFLAGS) $(SERVER_OFILES) -o myserver

clean:
	/bin/rm -f ./client/*.o ./server/*.o *~

