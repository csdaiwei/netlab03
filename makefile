CFLAGS = -MD -Wall -Werror -O2 -I./include
CFILES = $(shell find src/ -name "*.c")
OBJS = $(CFILES:.c=.o)

#objects for server
SOBJS = src/server.o src/readn.o	
#objects for client
COBJS = src/client.o src/readn.o

all: server client

server:$(SOBJS)
	gcc $(SOBJS) -o server

client:$(COBJS)
	gcc $(COBJS) -o client

# other dependencies
$(OBJS):include/bool.h
$(SOBJS):include/server.h
$(COBJS):include/client.h

.PHONY:clean
clean:
	rm -f server client $(OBJS) $(OBJS:.o=.d)
	find -name "*~" | xargs rm -f
