CC = gcc
CFLAGS = -Wall -O2 -pthread -I./include #-Werror 

#objects for both server and client 
OBJS = src/readn.o src/queue.o src/keyboard.o
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
	rm -f server client src/server.o src/client.o $(OBJS)
