#include <ncurses.h>

#define ROW_ZERO 2
#define COL_ZERO 5

#define ROW_MAX 15
#define COL_MAX 25

#define ALL_ACTORS_SIZE 32
#define ICON_PLAYER '@'

#define BUTTON_QUIT 'q'

#define DEBUG_PRINT 0

struct actor {
	int row;
	int col;
	char icon;
	const char name[32];
};

void draw_layer_terrain(void)
{
	move(ROW_ZERO, COL_ZERO);
	for (int i = 0; i < ROW_MAX; i++) {
		for (int k = 0; k < COL_MAX; k++) {
			move(ROW_ZERO + i, COL_ZERO + k);
			addch('.');
		}
	}
}

void draw_layer_actors(const struct actor ** const all_actors)
{
#if DEBUG_PRINT
	printw("all_actors[0] is: %08x\n", all_actors[0]);
	printw("all_actors[1] is: %08x\n", all_actors[1]);
#endif

	for (int i = 0; i < ALL_ACTORS_SIZE; i++) {
		if (0 != all_actors[i]) {
			move(all_actors[i]->row, all_actors[i]->col);
			addch(all_actors[i]->icon);
		}
	}
}

void draw(const struct actor ** const all_actors)
{
	draw_layer_terrain();
	draw_layer_actors(all_actors);
	refresh();
}

void initialize_io(void)
{
	initscr();
	noecho(); // Do not echo read chars
	cbreak(); // Read input contiguously
	keypad(stdscr, TRUE); // Read more keys, including arrow keys
}

int main(void) {
	int pressed_key = 0;

	const struct actor* all_actors[ALL_ACTORS_SIZE] = {0};
	struct actor player = {
		.row = ROW_ZERO + 2,
		.col = COL_ZERO + 2,
		.icon = ICON_PLAYER,
		.name = "wizard",
	};

	all_actors[0] = &player;

	initialize_io();

	while (BUTTON_QUIT != pressed_key) {
		draw(all_actors);
		pressed_key = getch();
	}

	endwin();

	return 0;
}
