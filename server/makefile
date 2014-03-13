CFLAGS = -Wall -Werror -O2
CFILES = $(shell find -name "*.c")
OBJS = $(CFILES:.c=.o)

server:server.c
	gcc server.c -o server 


.PHONY:clean
clean:
	rm -f server $(OBJS)
