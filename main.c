#include <ncurses.h>

#define ROW_ZERO 2
#define COL_ZERO 5

int main(void) {
	initscr();
	noecho(); // Do not echo read chars
	cbreak(); // Read input contiguously

	move(ROW_ZERO, COL_ZERO);
	for (int i = 0; i < 10; i++) {
		printw("..........");
		move(ROW_ZERO + i, COL_ZERO);
	}
	refresh();

	getch();
	endwin();

	return 0;
}
