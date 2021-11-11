#include <ncurses.h>

#define ROW_ZERO 2
#define COL_ZERO 5

#define ROW_MAX 15
#define COL_MAX 25

void draw_layer_terrain(void) {
	move(ROW_ZERO, COL_ZERO);
	for (int i = 0; i < ROW_MAX; i++) {
		for (int k = 0; k < COL_MAX; k++) {
			move(ROW_ZERO + i, COL_ZERO + k);
			addch('.');
		}
	}
}

void draw(void) {
	draw_layer_terrain();
	refresh();
}

int main(void) {
	initscr();
	noecho(); // Do not echo read chars
	cbreak(); // Read input contiguously

	draw();

	getch();
	endwin();

	return 0;
}
