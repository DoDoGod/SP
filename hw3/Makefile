CC=gcc
CFLAGS=-O2 -std=gnu99

TARGET=server file_reader sleep_cgi

all: $(TARGET)

server: server.c utility.h
	$(CC) $(CFLAGS) $< -o $@

file_reader: file_reader.c
	$(CC) $(CFLAGS) $< -o $@

sleep_cgi: file_reader.c
	$(CC) $(CFLAGS) -D SLEEP $< -o $@
	
