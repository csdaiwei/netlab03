CFLAGS = -Wall -Werror -O2 -I./include

.PHONY:all server client clean

all:server client

server: src/server.c
	gcc $(CFLAGS) src/server.c -o server 

client: src/client.c
	gcc $(CFLAGS) src/client.c -o client 

clean:
	rm -rf server client
	find -name "*~" | xargs rm -f


