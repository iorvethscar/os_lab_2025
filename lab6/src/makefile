CC = gcc
CFLAGS = -I.

all: server client

multmodulo.o: multmodulo.c multmodulo.h
	$(CC) -c multmodulo.c -o multmodulo.o $(CFLAGS)

libmultmodulo.a: multmodulo.o
	ar rcs libmultmodulo.a multmodulo.o

server: server.c libmultmodulo.a
	$(CC) server.c -o server -L. -lmultmodulo -pthread $(CFLAGS)

client: client.c libmultmodulo.a
	$(CC) client.c -o client -L. -lmultmodulo -pthread $(CFLAGS)

clean:
	rm -f server client multmodulo.o libmultmodulo.a
