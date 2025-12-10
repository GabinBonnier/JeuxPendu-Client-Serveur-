all: client server

client: client.c game.o
	gcc -o client client.c game.o

server: server.c game.o
	gcc -o server server.c game.o

game.o: game.c game.h
	gcc -c game.c

clean:
	rm -f *.o client server
