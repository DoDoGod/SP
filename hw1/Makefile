all: server.c
	gcc -O2 server.c -o write_server
	gcc -O2 server.c -D READ_SERVER -o read_server

clean:
	rm -f read_server write_server
