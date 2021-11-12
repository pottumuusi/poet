#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#define ROW_ZERO 2
#define COL_ZERO 5

#define ROW_ANNOUNCE_ZERO 35
#define COL_ANNOUNCE_ZERO 5

#define ROW_DEBUG_ZERO 45
#define COL_DEBUG_ZERO 5

#define ROW_MAX 15
#define COL_MAX 25

#define ALL_TERRAINS_SIZE 16
#define ALL_TERRAINS_FLOOR 0
#define ALL_TERRAINS_WALL_VERTICAL 1
#define ALL_TERRAINS_WALL_HORIZONTAL 2
#define ALL_TERRAINS_COLUMN 3

#define ALL_ACTORS_SIZE 32
#define ALL_ACTORS_PLAYER 0
#define ICON_PLAYER '@'
#define ICON_ITEM_DROP ';'

#define ACTOR_INVENTORY_SIZE 64
#define ALL_ITEMS_SIZE ACTOR_INVENTORY_SIZE * ALL_ACTORS_SIZE * 2

#define BUTTON_QUIT 'q'

#define DEBUG_PRINT_ENABLE 0

#if 0
move(ROW_DEBUG_ZERO, COL_DEBUG_ZERO);
#endif

struct actor {
	int row;
	int col;
	char icon;
	char name[32];
	int all_actors_index;
	int hitpoints;
	void (*on_interact) (struct actor* const self, struct actor* const other);
	struct item* inventory[ACTOR_INVENTORY_SIZE];
};

struct terrain {
	char icon;
	const char name[32];
	char traversable;
};

struct stage_shard {
	struct terrain* terrain;
	struct actor* occupant;
};

struct item {
	char consumable;
	char name[32];
	int all_items_index;
};

struct stage_shard stage[ROW_MAX][COL_MAX] = {0};
struct actor* all_actors[ALL_ACTORS_SIZE] = {0};

void draw_layer_terrain(void)
{
	move(ROW_ZERO, COL_ZERO);
	for (int i = 0; i < ROW_MAX; i++) {
		for (int k = 0; k < COL_MAX; k++) {
			move(ROW_ZERO + i, COL_ZERO + k);
			addch(stage[i][k].terrain->icon);
		}
	}
}

void draw_layer_actors(struct actor ** const all_actors)
{
	int row;
	int col;

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

int is_traversable_terrain(int row, int col)
{
	return stage[row][col].terrain->traversable;
}

int is_occupied(int row, int col)
{
	return 0 != stage[row][col].occupant;
}

void announce(const char* new_announcement) {
	// TODO announcement history viewing
	char space_row[128];
	static char announcements_shortlist[5][128];

	for (int i = 0; i < 128; i++) {
		space_row[i] = ' ';
	}
	space_row[128 - 1] = '\0';

	for (int i = 4; i >= 0; i--) {
		strcpy(announcements_shortlist[i + 1], announcements_shortlist[i]);
	}
	strcpy(announcements_shortlist[0], new_announcement);

	move(ROW_ANNOUNCE_ZERO, COL_ANNOUNCE_ZERO);
	for (int i = 0; i < 5; i++) {
		move(ROW_ANNOUNCE_ZERO + i, COL_ANNOUNCE_ZERO);
		printw(space_row);
		move(ROW_ANNOUNCE_ZERO + i, COL_ANNOUNCE_ZERO);
		printw(announcements_shortlist[i]);
	}
}

void interact_with_occupant_of(int row, int col, struct actor* const initiator)
{
	char str[128];
	struct actor* self = 0;

	self = stage[row][col].occupant;
	strcpy(str, initiator->name);
	strcat(str, " interacts with ");
	strcat(str, self->name);
	announce(str);
	stage[row][col].occupant->on_interact(self, initiator);
}

void update_position(
		const int pressed_key,
		struct actor* const actor)
{
	int new_position_row = actor->row;
	int new_position_col = actor->col;

	if (KEY_DOWN == pressed_key) {
		new_position_row = actor->row + 1;
	}

	if (KEY_UP == pressed_key) {
		new_position_row = actor->row - 1;
	}

	if (KEY_LEFT == pressed_key) {
		new_position_col = actor->col - 1;
	}

	if (KEY_RIGHT == pressed_key) {
		new_position_col = actor->col + 1;
	}

	if (!is_traversable_terrain(new_position_row, new_position_col)) {
		return;
	}

	if (is_occupied(new_position_row, new_position_col)) {
		interact_with_occupant_of(new_position_row, new_position_col, actor);
		return;
	}

	stage[actor->row][actor->col].occupant = 0;
	actor->row = new_position_row;
	actor->col = new_position_col;
	stage[actor->row][actor->col].occupant = actor;
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

int is_horizontal_edge(int i, int k)
{
	if (0 == i) {
		return 1;
	}

	if (ROW_MAX - 1 == i) {
		return 1;
	}

	return 0;
}

int is_vertical_edge(int i, int k)
{
	if (0 == k) {
		return 1;
	}

	if (COL_MAX - 1 == k) {
		return 1;
	}

	return 0;
}

void load_stage(struct terrain ** const all_terrains) {
	for (int i = 0; i < ROW_MAX; i++) {
		for (int k = 0; k < COL_MAX; k++) {
			if (is_corner(i, k)) {
				stage[i][k].terrain = all_terrains[ALL_TERRAINS_COLUMN];
			} else if (is_horizontal_edge(i, k)) {
				stage[i][k].terrain = all_terrains[ALL_TERRAINS_WALL_HORIZONTAL];
			} else if (is_vertical_edge(i, k)) {
				stage[i][k].terrain = all_terrains[ALL_TERRAINS_WALL_VERTICAL];
			} else {
				stage[i][k].terrain = all_terrains[ALL_TERRAINS_FLOOR];
			}
		}
	}
}

void add_to_inventory(struct actor* const self, struct actor* const initiator)
{
	int i = 0;
	char str[128];

	strcpy(str, "add_to_inventory()");
	announce(str);

	// move inventory content
	// while (0 != self->inventory[i]) {
	// }

	stage[self->row][self->col].occupant = 0;
	all_actors[self->all_actors_index] = 0;
	free(self);
}

void spawn_item_consumable(struct item ** const all_items, int first_free)
{
	all_items[first_free] = malloc(sizeof(struct item));
	all_items[first_free]->consumable = 1;
	strcpy(all_items[first_free]->name, "potion");
	all_items[first_free]->all_items_index = first_free;
}

void spawn_item_equipment(struct item ** const all_items, int first_free)
{
	all_items[first_free] = malloc(sizeof(struct item));
	all_items[first_free]->consumable = 0;
	strcpy(all_items[first_free]->name, "dagger");
	all_items[first_free]->all_items_index = first_free;
}

int spawn_item(struct item ** const all_items, int quality, int* new_item_index)
{
	int first_free = -1;

	for (int i = 0; i < ALL_ITEMS_SIZE; i++) {
		if (0 == all_items[i]) {
			first_free = i;
		}
	}

	if (-1 == first_free) {
		return -1;
	}

	if (0 == random() % 2) {
		spawn_item_consumable(all_items, first_free);
	} else {
		spawn_item_equipment(all_items, first_free);
	}

	return 0;
}

void spawn_item_drop(struct actor ** const all_actors, struct item ** const all_items, int quality)
{
	int ret = 0;
	int row = 0;
	int col = 0;
	int first_free = -1;
	int new_item_index = -1;

	for (int i = 0; i < ALL_ACTORS_SIZE; i++) {
		if (0 == all_actors[i]) {
			first_free = i;
		}
	}

	if (-1 == first_free) {
		// all slots full, bail out
		return;
	}

	ret = spawn_item(all_items, quality, &new_item_index);
	if (0 != ret) {
		// all slots full, bail out
		return;
	}

	all_actors[first_free] = malloc(sizeof(struct actor)); // TODO free memory and mark empty on pickup
	all_actors[first_free]->row = 4;
	all_actors[first_free]->col = 4;
	all_actors[first_free]->icon = ICON_ITEM_DROP;
	strcpy(all_actors[first_free]->name, "item drop");
	all_actors[first_free]->all_actors_index = first_free;
	all_actors[first_free]->on_interact = add_to_inventory;
	all_actors[first_free]->inventory[0] = all_items[new_item_index];

	for (int i = 0; i < ACTOR_INVENTORY_SIZE; i++) {
		all_actors[first_free]->inventory[i] = 0;
	}

	row = all_actors[first_free]->row;
	col = all_actors[first_free]->col;
	stage[row][col].occupant = all_actors[first_free];
}

void initialize_io(void)
{
	initscr();
	noecho(); // Do not echo read chars
	cbreak(); // Read input contiguously
	keypad(stdscr, TRUE); // Read more keys, including arrow keys
	curs_set(0);
}

int main(void) {
	int pressed_key = 0;

	struct terrain* all_terrains[ALL_TERRAINS_SIZE] = {0};
	struct item* all_items[ALL_ITEMS_SIZE] = {0};
	struct actor player = {
		.row = 2,
		.col = 2,
		.icon = ICON_PLAYER,
		.name = "wizard",
	};

	struct terrain floor = {
		.icon = '.',
		.name = "floor",
		.traversable = 1,
	};

	struct terrain wall_vertical = {
		.icon = '|',
		.name = "wall_vertical",
		.traversable = 0,
	};

	struct terrain wall_horizontal = {
		.icon = '-',
		.name = "wall_horizontal",
		.traversable = 0,
	};

	struct terrain column = {
		.icon = '+',
		.name = "column",
		.traversable = 0,
	};

	all_actors[ALL_ACTORS_PLAYER]			= &player;

	all_terrains[ALL_TERRAINS_FLOOR]		= &floor;
	all_terrains[ALL_TERRAINS_WALL_VERTICAL]	= &wall_vertical;
	all_terrains[ALL_TERRAINS_WALL_HORIZONTAL]	= &wall_horizontal;
	all_terrains[ALL_TERRAINS_COLUMN]		= &column;

	initialize_io();
	load_stage(all_terrains);

	spawn_item_drop(all_actors, all_items, 2);

	while (BUTTON_QUIT != pressed_key) {
		draw(all_actors);
		pressed_key = getch();
		update(pressed_key, all_actors);
	}

	endwin();

	return 0;
}
