all: server client

server: server.c socketlayer.c socketprx.h
	gcc -Wall server.c -o server -lpthread socketlayer.c
client: client.c socketlayer.c socketprx.h
	gcc -Wall client.c -o client -lpthread socketlayer.c

clean:
	rm -f client client.o server server.o

