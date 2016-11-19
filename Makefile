CC = gcc

game: fbdisplay.h game.c
	$(CC) -o game game.c
