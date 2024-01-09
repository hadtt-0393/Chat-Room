all: server client

allv2: server clientv2

server: server.c account_manager.c error.c util.c
	gcc -o server server.c account_manager.c error.c util.c

client: client.c error.c util.c
	gcc -o client client.c error.c util.c

clientv2: client.v2.c error.c util.c
	gcc -o client client.v2.c error.c util.c -lcurses