CC := gcc

all: game

game:
	$(CC) -lncurses main.c
	mv ./a.out collector
