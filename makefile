CC = gcc
CFLAGS = -Wall -O2 -pthread -I./include #-Werror 

#objects for server
SOBJS = src/server.o src/readn.o	
#objects for client
COBJS = src/client.o src/readn.o
#all objects
OBJS = src/client.o src/server.o src/readn.o

all: server client

server:$(SOBJS)
	$(CC) $(CFLAGS)	 -o server $(SOBJS) 

client:$(COBJS)
	$(CC) $(CFLAGS)	 -o client $(COBJS) 

# other dependencies
$(OBJS):include/bool.h include/protocol.h
$(SOBJS):include/server.h
$(COBJS):include/client.h

.PHONY:clean
clean:
	rm -f server client $(OBJS)
	find -name "*~" | xargs rm -f
