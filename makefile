all: client server copy

client: client.o
	gcc -o client client.o

copy: 
	@echo "Copying client to /tmp"
	cp -f client /tmp/client

server: server.o
	gcc -o server server.o

client.o: client.c client.h
	gcc -c client.c

server.o: server.c server.h
	gcc -c server.c

clean:
	rm *.o
