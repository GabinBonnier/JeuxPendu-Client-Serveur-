all: client1 client2 server

client1: client1.c game.o
	gcc -o client1 client1.c game.o

client2: client2.c game.o
	gcc -o client2 client2.c game.o

server: server.c game.o
	gcc -o server server.c game.o

game.o: game.c game.h
	gcc -c game.c

clean:
	rm -f *.o client1 client2 server
