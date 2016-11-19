CC = gcc

game: fbdisplay.h EasyPIO.h game.c
	$(CC) -o game game.c
