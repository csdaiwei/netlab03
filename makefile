CC = gcc
CFLAGS = -Wall -O2 -pthread -I./include #-Werror 

#objects for server
SOBJS = src/server.o src/readn.o src/queue.o
#objects for client
COBJS = src/client.o src/readn.o src/queue.o

all: server client

server:$(SOBJS)
	$(CC) $(CFLAGS)	 -o server $(SOBJS) 

client:$(COBJS)
	$(CC) $(CFLAGS)	 -o client $(COBJS) 


.PHONY:clean
clean:
	rm -f server client $(COBJS) $(SOBJS)
	find -name "*~" | xargs rm -f
