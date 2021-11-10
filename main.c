#include <ncurses.h>

int main(void) {
	initscr();
	move(2, 5);
	printw("Hello World!");
	refresh();
	getch();
	endwin();

	return 0;
}
