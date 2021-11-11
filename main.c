#include <ncurses.h>

#define ROW_ZERO 2
#define COL_ZERO 5

#define ROW_DEBUG_ZERO 35
#define COL_DEBUG_ZERO 5

#define ROW_MAX 15
#define COL_MAX 25

#define ALL_ACTORS_SIZE 32
#define ALL_ACTORS_PLAYER_INDEX 0
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

void draw_layer_actors(struct actor ** const all_actors)
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

void draw(struct actor ** const all_actors)
{
	draw_layer_terrain();
	draw_layer_actors(all_actors);
	refresh();
}

int is_position_button(const int pressed_key)
{
	if (KEY_DOWN == pressed_key) {
		return 1;
	}

	if (KEY_UP == pressed_key) {
		return 1;
	}

	if (KEY_LEFT == pressed_key) {
		return 1;
	}

	if (KEY_RIGHT == pressed_key) {
		return 1;
	}

	if (' ' == pressed_key) {
		return 1;
	}

	return 0;
}

void update_position(
		const int pressed_key,
		struct actor* const actor)
{
	// TODO collision detection

	if (KEY_DOWN == pressed_key) {
		actor->row = actor->row + 1;
	}

	if (KEY_UP == pressed_key) {
		actor->row = actor->row - 1;
	}

	if (KEY_LEFT == pressed_key) {
		actor->col = actor->col - 1;
	}

	if (KEY_RIGHT == pressed_key) {
		actor->col = actor->col + 1;
	}
}

void update_player(const int pressed_key, struct actor* const player_actor)
{
	if (is_position_button(pressed_key)) {
		update_position(pressed_key, player_actor);
	}
}

void update(
		const int pressed_key,
		struct actor ** const all_actors)
{
	update_player(pressed_key, all_actors[ALL_ACTORS_PLAYER_INDEX]);
	// Player and UI related updates dependent on user input.
	// Enemy updates independent of user input.
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

	struct actor* all_actors[ALL_ACTORS_SIZE] = {0};
	struct actor player = {
		.row = ROW_ZERO + 2,
		.col = COL_ZERO + 2,
		.icon = ICON_PLAYER,
		.name = "wizard",
	};

	all_actors[ALL_ACTORS_PLAYER_INDEX] = &player;

	initialize_io();

	while (BUTTON_QUIT != pressed_key) {
		draw(all_actors);
		pressed_key = getch();
		update(pressed_key, all_actors);
	}

	endwin();

	return 0;
}
