CC=g++
CFLAGS=-std=c++11

all: serverM serverA serverB client

serverM: serverM.c
	$(CC) $(CFLAGS) -o serverM serverM.c

serverA: serverA.c
	$(CC) $(CFLAGS) -o serverA serverA.c

serverB: serverB.c
	$(CC) $(CFLAGS) -o serverB serverB.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f serverM serverA serverB client