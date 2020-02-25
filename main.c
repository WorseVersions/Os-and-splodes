#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define OW 'O'
#define EMPT '.'
#define PLAYER '@'

#define OWCOL 1
#define EMPTCOL 2
#define PLAYERCOL 3
#define WINCOL 4
#define LOSECOL 5
void initColors() {
	if (has_colors() == FALSE) {
		endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	} else if (has_colors() == TRUE) {
		start_color();
		init_pair(OWCOL, COLOR_YELLOW, COLOR_YELLOW);
		init_pair(EMPTCOL, COLOR_BLACK, COLOR_WHITE);
		init_pair(PLAYERCOL, COLOR_BLACK, COLOR_WHITE);
		init_pair(WINCOL, COLOR_WHITE, COLOR_YELLOW);
		init_pair(LOSECOL, COLOR_WHITE, COLOR_RED);
	}
}

void drawMap() {
	mvaddch(1,0, '+');
	mvaddch(LINES-1,0, '+');
	mvaddch(1,COLS-1, '+');
	mvaddch(LINES-1, COLS-1, '+');
	mvhline(1, 1, '-', COLS-2);
	mvhline(LINES-1, 1, '-', COLS-2); 
	mvvline(2, 0, '|', LINES-3);
	mvvline(2, COLS-1, '|', LINES-3);
}

void checkSurr(int x, int y, int *surr) {
	surr[0] = mvinch(y, x+1) & ~(A_COLOR | A_BOLD);
	surr[1] = mvinch(y, x-1) & ~(A_COLOR | A_BOLD);
	surr[2] = mvinch(y+1, x) & ~(A_COLOR | A_BOLD);
	surr[3] = mvinch(y-1, x) & ~(A_COLOR | A_BOLD);
	surr[4] = mvinch(y+1, x+1) & ~(A_COLOR | A_BOLD);
	surr[5] = mvinch(y-1, x-1) & ~(A_COLOR | A_BOLD);
	surr[6] = mvinch(y-1, x+1) & ~(A_COLOR | A_BOLD);
	surr[7] = mvinch(y+1, x-1) & ~(A_COLOR | A_BOLD);
}

/* thanks StackOverflow! ^_^ */
int randlim(int limit) {
    int divisor = RAND_MAX/(limit+1);
    int retval;

    do { 
        retval = rand() / divisor;
    } while (retval > limit);

    return retval;
}

int drawO() {
	int num = randlim((LINES*COLS)-5);
	int x = 0, y = 0;
	for (int i = 0; i < num; i++) {
		x = randlim(COLS-4)+1;
		y = randlim(LINES-4)+2;
		attron(COLOR_PAIR(OWCOL) | A_BOLD);
		mvaddch(y, x, OW);
	}
	return num;
}

void splode(int x, int y, char ch) {
	if (ch == EMPT) 
		attron(COLOR_PAIR(EMPTCOL) | A_BOLD);
	else if (ch == OW)
		attron(COLOR_PAIR(OWCOL) | A_BOLD);
	else
		attron(A_NORMAL | A_BOLD);
		
	mvhline(y-1, x-1, ch, 2);
	mvhline(y+1, x-1, ch, 2);
	mvvline(y-1, x-1, ch, 2);
	mvvline(y-1, x+1, ch, 2);
	attron(A_NORMAL | A_BOLD);
	drawMap();
}

void screen(char *message, char *message2, int num) {
	clear();
	attron(COLOR_PAIR(num) | A_BOLD);
	mvprintw(LINES/2, (COLS/2)-strlen(message), message);
	mvprintw((LINES/2)+2, (COLS/2)-26, "Press Space To Continue...");
	mvprintw(0,0," ");
	while(getch()!=' ')
		printw(message2);
		usleep(10);
}

int main() {
	initscr();
	cbreak();
	noecho();
	curs_set(FALSE);
	keypad(stdscr, TRUE);
	timeout(1);
	srand(time(0));
	int ch, x = 2, y = 2, score, ox, oy, splodes=randlim(10)+4;
	int ow = COLS, oh = LINES;
	int *surr = malloc(sizeof(int)*8);
	initColors();
	drawMap();
	int numO = drawO();
	bool hasWon = false;
	while (1) {
		/* game loop*/
		ch = getch();
		checkSurr(x,y, surr);
		if (ch == KEY_F(1) || ch == '`' || splodes == 0) { /* if f1 is pressed exit*/
			break;
		} else if ((ch == 'w' || ch == KEY_UP) && surr[3] != '-' && surr[3] != EMPT) {
			y--;
		} else if ((ch == 's' || ch == KEY_DOWN) && surr[2] != '-' && surr[2] != EMPT) {
			y++; 
		} else if ((ch == 'a' || ch == KEY_LEFT) && surr[1] != '|' && surr[1] != EMPT) {
			x--;
		} else if ((ch == 'd' || ch == KEY_RIGHT) && surr[0] != '|' && surr[0] != EMPT) {
			x++;
		} else if (ch == 'e' && splodes >= 1) {
			splodes--;
			splode(x, y, ' ');
		} else if (ch == 'r' && splodes >= 1) {
			splodes--;
			splode(x,y, EMPT);
		} else if (ch == 't' && splodes >= 3) {
			splodes -= 3;
			splode(x,y, OW);
		}
		if (ow != COLS || oh != LINES) {
			clear();
			drawMap();
			numO = drawO();
			ow = COLS;
			oh = LINES;
			if (x > ow || y > oh) {
				x = 2;
				y = 2;
			}
			attron(COLOR_PAIR(PLAYERCOL) | A_BOLD);
			mvaddch(y,x, PLAYER);
		}
		if (ox != x || oy != y) {
			attron(COLOR_PAIR(EMPTCOL) | A_BOLD);
			mvaddch(oy, ox, ' ');
			ox = x;
			oy = y;
			attron(COLOR_PAIR(PLAYERCOL) | A_BOLD);
			mvaddch(y, x, '@');	
		}
		checkSurr(x,y, surr);
		if (surr[0] == OW) {
			score++;
			attron(COLOR_PAIR(EMPTCOL) | A_BOLD);
			mvaddch(y, x+1, EMPT);
		} else if (surr[1] == OW) {
			score++;
			attron(COLOR_PAIR(EMPTCOL) | A_BOLD);
			mvaddch(y, x-1, EMPT);
		} else if (surr[2] == OW) {
			score++;
			attron(COLOR_PAIR(EMPTCOL) | A_BOLD);
			mvaddch(y+1, x, EMPT);
		} else if (surr[3] == OW) {
			score++;
			attron(COLOR_PAIR(EMPTCOL) | A_BOLD);
			mvaddch(y-1, x, EMPT);
		} else if (surr[4] == OW) {
			score++;
			attron(COLOR_PAIR(EMPTCOL) | A_BOLD);
			mvaddch(y+1, x+1, EMPT);
		} else if (surr[5] == OW) {
			score++;
			attron(COLOR_PAIR(EMPTCOL) | A_BOLD);
			mvaddch(y-1, x-1, EMPT);
		} else if (surr[6] == OW) {
			score++; 
			attron(COLOR_PAIR(EMPTCOL) | A_BOLD); 
			mvaddch(y-1, x+1, EMPT);
		} else if (surr[7] == OW) {
			score++;
			attron(COLOR_PAIR(EMPTCOL) | A_BOLD);
			mvaddch(y+1, x-1, EMPT);
		}
		attron(COLOR_PAIR(EMPTCOL) | A_BOLD);
		mvprintw(0,0,"Score: %d/%d", score, numO);
		mvprintw(0, 20, "Splodes: %d", splodes);
		attron(A_NORMAL | A_BOLD);
		if (score == numO || ch==KEY_F(12)) {
			hasWon = true;
			break;
		}
	}
	if (hasWon) {
		screen("You Win!", "\\Win/", WINCOL);
	} else {
		screen("You lose", "\\Bye/", LOSECOL);
	}
	endwin();
	return 0;
}
