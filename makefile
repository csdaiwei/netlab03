CC = gcc
CFLAGS = -Wall -Werror -O2 -pthread -I./include  

#objects for both server and client 
OBJS = src/network.o src/queue.o src/keyboard.o
#objects for server
SOBJS = src/server.o $(OBJS)
#objects for client
COBJS = src/client.o $(OBJS)

all: server client

server:$(SOBJS)
	$(CC) $(CFLAGS)	 -o server $(SOBJS) 

client:$(COBJS)
	$(CC) $(CFLAGS)	 -o client $(COBJS) 

.PHONY:clean
clean:
	rm -f server client 
	rm -f src/server.o src/client.o 
	rm -f $(OBJS)
