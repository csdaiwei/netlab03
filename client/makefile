CFLAGS = -Wall 
CFILES = $(shell find -name "*.c")
OBJS = $(CFILES:.c=.o)

client: client.c
	gcc -o client client.c

.PHONY:clean
clean:
	rm -f client $(OBJS)
