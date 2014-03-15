CC = gcc
CFLAGS = -Wall -Werror -O2 -I./include


#objects for server
SOBJS = src/server.o src/readn.o	
#objects for client
COBJS = src/client.o src/readn.o

all: server client

server:$(SOBJS)
	$(CC) $(CFLAGS) $(SOBJS) -o server

client:$(COBJS)
	$(CC) $(CFLAGS) $(COBJS) -o client

# other dependencies
$(OBJS):include/bool.h include/protocol.h
$(SOBJS):include/server.h
$(COBJS):include/client.h

.PHONY:clean
clean:
	rm -f server client src/*.o
	find -name "*~" | xargs rm -f
