all : server client
.PHONY : all
server : server.c
	gcc -Wall -O2 server.c -o server
client : client.c
	gcc -Wall -O2 client.c -o client
clean :
	rm server receive_file
