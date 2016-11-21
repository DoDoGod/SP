CC = gcc
CFLAGS = -std=c99 -O2
TARGET = big_judge judge player

all: $(TARGET)

%: %.c
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm -rf bid_judge judge player
