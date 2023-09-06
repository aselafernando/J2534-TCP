CXX = g++
CC = gcc
CXXFLAGS += -I./J2534_TCP/include

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CXXFLAGS +=-DDEBUG -O0 -g
else
    CXXFLAGS +=-DNDEBUG -O2
endif

SERVER  = J2534_TCPServer
SERVER_SOURCES = J2534_TCP/server/J2534.cpp J2534_TCP/server/J2534_Server.cpp J2534_TCP/server/J2534_TCPServer.cpp
SERVER_OBJECTS = $(SERVER_SOURCES:.cpp=.o)

CLIENT = libJ2534.so
CLIENT_SOURCES = J2534_TCP/client/J2534_Client.cpp J2534_TCP/client/dllmain.cpp J2534_TCP/client/stdafx.cpp
CLIENT_OBJECTS = $(CLIENT_SOURCES:.cpp=.o)

all: $(SERVER) $(CLIENT)

${SERVER} : $(SERVER_OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(SERVER) $(SERVER_OBJECTS) -ldl -lpthread

J2534_TCP/client/%.o : J2534_TCP/client/%.cpp | ./J2534_TCP/client
	$(CXX) -c $(CXXFLAGS) -fPIC -fvisibility=hidden -o $@ $<

${CLIENT} : $(CLIENT_OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $(CLIENT) $(CLIENT_OBJECTS)

clean:
	rm $(SERVER) $(CLIENT) $(SERVER_OBJECTS) $(CLIENT_OBJECTS)
