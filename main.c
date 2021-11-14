#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ROW_STAGE_ZERO 2
#define COL_STAGE_ZERO 5

#define ROW_MAX 15
#define COL_MAX 25

#define ROW_ANNOUNCE_ZERO 35
#define COL_ANNOUNCE_ZERO 5

#define ROW_HUD_ZERO ROW_STAGE_ZERO
#define COL_HUD_ZERO COL_MAX + COL_STAGE_ZERO + 3

#define ROW_DEBUG_ZERO 45
#define COL_DEBUG_ZERO 5

#define ALL_TERRAINS_SIZE 16
#define ALL_TERRAINS_FLOOR 0
#define ALL_TERRAINS_WALL_VERTICAL 1
#define ALL_TERRAINS_WALL_HORIZONTAL 2
#define ALL_TERRAINS_COLUMN 3

#define ALL_ACTORS_SIZE 32
#define ALL_ACTORS_PLAYER 0
#define ICON_PLAYER '@'
#define ICON_ITEM_DROP ';'

#define HUD_DRAW_HIDE 0
#define HUD_DRAW_INVENTORY 1

#define HUD_ROWS 24

#define ACTOR_INVENTORY_SIZE 64
#define ALL_ITEMS_SIZE ACTOR_INVENTORY_SIZE * ALL_ACTORS_SIZE * 2

#define BUTTON_QUIT 'q'

#define ANNOUNCEMENT_SIZE 128
#define DEBUG_MESSAGE_SIZE 128

#define DEBUG_PRINT_ENABLE 0

#if 0
move(ROW_DEBUG_ZERO + row_debug_current, COL_DEBUG_ZERO);
row_debug_current++;
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
	int amount;
	char consumable;
	char name[32];
	int all_items_index;
};

struct stage_shard stage[ROW_MAX][COL_MAX] = {0};
struct actor* all_actors[ALL_ACTORS_SIZE] = {0};

int row_debug_current = 0;

int hud_to_draw = 0;

int get_first_free_inventory_slot(struct item** const inventory)
{
	for (int i = 0; i < ACTOR_INVENTORY_SIZE; i++) {
		if (0 == inventory[i]) {
			return i;
		}
	}

	return -1;
}

int get_first_free_actor_slot(struct actor** const actors)
{
	for (int i = 0; i < ALL_ACTORS_SIZE; i++) {
		if (0 == all_actors[i]) {
			return i;
		}
	}

	return -1;
}

int get_first_free_item_slot(struct item** const items)
{
	return get_first_free_inventory_slot(items);
}

void draw_layer_terrain(void)
{
	move(ROW_STAGE_ZERO, COL_STAGE_ZERO);
	for (int i = 0; i < ROW_MAX; i++) {
		for (int k = 0; k < COL_MAX; k++) {
			move(ROW_STAGE_ZERO + i, COL_STAGE_ZERO + k);
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
			row = ROW_STAGE_ZERO + all_actors[i]->row;
			col = COL_STAGE_ZERO + all_actors[i]->col;
			move(row, col);
			addch(all_actors[i]->icon);
		}
	}
}

void draw_hud_hide(void)
{
	char space_row[128];

	for (int i = 0; i < 128; i++) {
		space_row[i] = ' ';
	}
	space_row[128 - 1] = '\0';

	for (int i = 0; i < HUD_ROWS + 1; i++) {
		move(ROW_HUD_ZERO + i, COL_HUD_ZERO);
		printw(space_row);
	}
}

void draw_hud_inventory(struct item** inventory)
{
	move(ROW_HUD_ZERO, COL_HUD_ZERO);
	printw("INVENTORY");
	for (int i = 0; i < HUD_ROWS; i++) {
		if (0 == inventory[i]) {
			break;
		}

		move(ROW_HUD_ZERO + 1 + i, COL_HUD_ZERO);
		printw(inventory[i]->name);
	}
}

void draw_layer_hud()
{
	switch(hud_to_draw) {
	case HUD_DRAW_HIDE:
		draw_hud_hide();
		break;
	case HUD_DRAW_INVENTORY:
		draw_hud_inventory(all_actors[ALL_ACTORS_PLAYER]->inventory);
		break;
	}
}

void draw(struct actor ** const all_actors)
{
	draw_layer_terrain();
	draw_layer_actors(all_actors);
	draw_layer_hud();
	refresh();
}

int is_position_button(int* const pressed_key)
{
	if (KEY_DOWN == *pressed_key) {
		return 1;
	}

	if (KEY_UP == *pressed_key) {
		return 1;
	}

	if (KEY_LEFT == *pressed_key) {
		return 1;
	}

	if (KEY_RIGHT == *pressed_key) {
		return 1;
	}

	if (' ' == *pressed_key) {
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
	char str[ANNOUNCEMENT_SIZE];
	struct actor* self = 0;

	self = stage[row][col].occupant;
	strcpy(str, initiator->name);
	strcat(str, " interacts with ");
	strcat(str, self->name);
	announce(str);
	stage[row][col].occupant->on_interact(self, initiator);
}

void update_position(
		int* const pressed_key,
		struct actor* const actor)
{
	int new_position_row = actor->row;
	int new_position_col = actor->col;

	if (KEY_DOWN == *pressed_key) {
		new_position_row = actor->row + 1;
	}

	if (KEY_UP == *pressed_key) {
		new_position_row = actor->row - 1;
	}

	if (KEY_LEFT == *pressed_key) {
		new_position_col = actor->col - 1;
	}

	if (KEY_RIGHT == *pressed_key) {
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

int is_hud_button(int* const pressed_key)
{
	if ('i' == *pressed_key) {
		return 1;
	}

	return 0;
}

void toggle_hud_inventory(void)
{
	if (HUD_DRAW_INVENTORY == hud_to_draw) {
		hud_to_draw = HUD_DRAW_HIDE;
		return;
	}

	hud_to_draw = HUD_DRAW_INVENTORY;
}

void toggle_hud(int* const pressed_key)
{
	if ('i' == *pressed_key) {
		toggle_hud_inventory();
	}
}

void update_hud(int* const pressed_key)
{
	if (is_hud_button(pressed_key)) {
		toggle_hud(pressed_key);
		// TODO consume button press
	}
}

void update_player(int* const pressed_key, struct actor* const player_actor)
{
	if (is_position_button(pressed_key)) {
		update_position(pressed_key, player_actor);
		// TODO consume button press
	}
}

void update(
		int* const pressed_key,
		struct actor ** const all_actors)
{
	// Player and UI related updates dependent on user input.
	update_hud(pressed_key);

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

void add_to_inventory(struct item* item_to_add, struct item** inventory)
{
	int first_free = -1;
	char str[ANNOUNCEMENT_SIZE];
#if 0
	char str_debug[DEBUG_MESSAGE_SIZE];

	strcpy(str, "add_to_inventory() adding item: ");
	strcat(str, item_to_add->name);
	move(ROW_DEBUG_ZERO + row_debug_current, COL_DEBUG_ZERO);
	row_debug_current++;
	printw("%s", str);
#endif

	first_free = get_first_free_inventory_slot(inventory);
	if (-1 == first_free) {
		strcpy(str, "Inventory full");
		announce(str);
		return;
	}

	inventory[first_free] = item_to_add;
}

void transfer_inventory_content(struct item** inventory_from, struct item** inventory_to)
{
	int i = 0;

	while (0 != inventory_from[i] && i < ACTOR_INVENTORY_SIZE) {
		add_to_inventory(inventory_from[i], inventory_to);
		inventory_from[0] = 0;
		i++;
	}
}

void get_picked(struct actor* const self, struct actor* const initiator)
{
#if 0
	char str[DEBUG_MESSAGE_SIZE];

	strcpy(str, "get_picked() called by: ");
	strcat(str, self->name);
	strcat(str, ", initiated by: ");
	strcat(str, initiator->name);
	move(ROW_DEBUG_ZERO + row_debug_current, COL_DEBUG_ZERO);
	row_debug_current++;
	printw("%s", str);
#endif

	transfer_inventory_content(self->inventory, initiator->inventory);

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

	first_free = get_first_free_item_slot(all_items);

	if (-1 == first_free) {
		return -1;
	}

	if (0 == random() % 2) {
		spawn_item_consumable(all_items, first_free);
	} else {
		spawn_item_equipment(all_items, first_free);
	}

	*new_item_index = first_free;

	return 0;
}

void spawn_item_drop(struct actor ** const all_actors, struct item ** const all_items, int quality)
{
	int ret = 0;
	int row = 0;
	int col = 0;
	int first_free = -1;
	int new_item_index = -1;

	first_free = get_first_free_actor_slot(all_actors);

	if (-1 == first_free) {
		// all slots full, bail out
		return;
	}

	ret = spawn_item(all_items, quality, &new_item_index);
	if (0 != ret) {
		// all slots full, bail out
		return;
	}

	all_actors[first_free] = malloc(sizeof(struct actor));
	for (int i = 0; i < ACTOR_INVENTORY_SIZE; i++) {
		all_actors[first_free]->inventory[i] = 0;
	}

	all_actors[first_free]->row = 4;
	all_actors[first_free]->col = 4;
	all_actors[first_free]->icon = ICON_ITEM_DROP;
	strcpy(all_actors[first_free]->name, "item drop");
	all_actors[first_free]->all_actors_index = first_free;
	all_actors[first_free]->on_interact = get_picked;
	all_actors[first_free]->inventory[0] = all_items[new_item_index];

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
	time_t t;

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

	all_terrains[ALL_TERRAINS_FLOOR]		= &floor;
	all_terrains[ALL_TERRAINS_WALL_VERTICAL]	= &wall_vertical;
	all_terrains[ALL_TERRAINS_WALL_HORIZONTAL]	= &wall_horizontal;
	all_terrains[ALL_TERRAINS_COLUMN]		= &column;

	all_actors[ALL_ACTORS_PLAYER]			= &player;
	for (int i = 0; i < ACTOR_INVENTORY_SIZE; i++) {
		player.inventory[i] = 0;
	}
	player.all_actors_index = ALL_ACTORS_PLAYER;

	srandom((unsigned) time(&t));

	initialize_io();
	load_stage(all_terrains);

	spawn_item_drop(all_actors, all_items, 2);

	while (BUTTON_QUIT != pressed_key) {
		draw(all_actors);
		pressed_key = getch();
		update(&pressed_key, all_actors);
	}

	endwin();

	return 0;
}
