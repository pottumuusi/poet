#include <ncurses.h>

#define ROW_ZERO 2
#define COL_ZERO 5

#define ROW_DEBUG_ZERO 35
#define COL_DEBUG_ZERO 5

#define ROW_MAX 15
#define COL_MAX 25

#define ALL_TERRAINS_SIZE 16
#define ALL_TERRAINS_FLOOR 0
#define ALL_TERRAINS_WALL_DIAGONAL 1
#define ALL_TERRAINS_WALL_HORIZONTAL 2
#define ALL_TERRAINS_COLUMN 3

#define ALL_ACTORS_SIZE 32
#define ALL_ACTORS_PLAYER 0
#define ICON_PLAYER '@'

#define BUTTON_QUIT 'q'

#define DEBUG_PRINT 0

struct actor {
	int row;
	int col;
	char icon;
	const char name[32];
};

struct terrain {
	char icon;
	const char name[32];
};

struct terrain* stage[ROW_MAX][COL_MAX];

void draw_layer_terrain(void)
{
	move(ROW_ZERO, COL_ZERO);
	for (int i = 0; i < ROW_MAX; i++) {
		for (int k = 0; k < COL_MAX; k++) {
			move(ROW_ZERO + i, COL_ZERO + k);
			addch(stage[i][k]->icon);
		}
	}
}

void draw_layer_actors(struct actor ** const all_actors)
{
	int row;
	int col;

#if DEBUG_PRINT
	printw("all_actors[0] is: %08x\n", all_actors[0]);
	printw("all_actors[1] is: %08x\n", all_actors[1]);
#endif

	for (int i = 0; i < ALL_ACTORS_SIZE; i++) {
		if (0 != all_actors[i]) {
			row = ROW_ZERO + all_actors[i]->row;
			col = COL_ZERO + all_actors[i]->col;
			move(row, col);
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
	// Player and UI related updates dependent on user input.
	update_player(pressed_key, all_actors[ALL_ACTORS_PLAYER]);

	// Enemy updates independent of user input.
}

int is_corner(int i, int k)
{
	if (0 == i && 0 == k) {
		return 1;
	}

	if (ROW_MAX - 1 == i && 0 == k) {
		return 1;
	}

	if (0 == i && COL_MAX - 1 == k) {
		return 1;
	}

	if (ROW_MAX - 1 == i && COL_MAX - 1 == k) {
		return 1;
	}

	return 0;
}

void load_stage(struct terrain ** const all_terrains) {
	for (int i = 0; i < ROW_MAX; i++) {
		for (int k = 0; k < COL_MAX; k++) {
			if (is_corner(i, k)) {
				stage[i][k] = all_terrains[ALL_TERRAINS_COLUMN];
			} else {
				stage[i][k] = all_terrains[ALL_TERRAINS_FLOOR];
			}
		}
	}
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
	struct terrain* all_terrains[ALL_TERRAINS_SIZE];
	struct actor player = {
		.row = 2,
		.col = 2,
		.icon = ICON_PLAYER,
		.name = "wizard",
	};

	struct terrain floor = {
		.icon = '.',
		.name = "floor",
	};

	struct terrain wall_diagonal = {
		.icon = '|',
		.name = "wall_diagonal",
	};

	struct terrain wall_horizontal = {
		.icon = '-',
		.name = "wall_horizontal",
	};

	struct terrain column = {
		.icon = '+',
		.name = "column",
	};

	all_actors[ALL_ACTORS_PLAYER]			= &player;

	all_terrains[ALL_TERRAINS_FLOOR]		= &floor;
	all_terrains[ALL_TERRAINS_WALL_DIAGONAL]	= &wall_diagonal;
	all_terrains[ALL_TERRAINS_WALL_HORIZONTAL]	= &wall_horizontal;
	all_terrains[ALL_TERRAINS_COLUMN]		= &column;

	initialize_io();
	load_stage(all_terrains);

	while (BUTTON_QUIT != pressed_key) {
		draw(all_actors);
		pressed_key = getch();
		update(pressed_key, all_actors);
	}

	endwin();

	return 0;
}
