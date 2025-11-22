CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lSDL2 -lSDL2_ttf -lm

all: tetris

tetris: main.c
	$(CC) $(CFLAGS) main.c -o tetris $(LIBS)

run: tetris
	clear
	./tetris

clean:
	rm -f tetris