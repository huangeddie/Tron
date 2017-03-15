all: server.o client.o

server.o: server.h server.c
	gcc -std=gnu99 -g -c -o server.o -std=gnu99 server.c -lpthread -Wall

client.o: client.h client.c
	gcc -std=gnu99 -g -c -o client.o -std=gnu99 client.c -lpthread -Wall

clean:
	-rm -rf *.o
