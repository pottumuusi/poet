#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ROW_STAGE_ZERO 3
#define COL_STAGE_ZERO 5

#define ROW_MAX 15
#define COL_MAX 25

#define ROW_ANNOUNCE_ZERO 35
#define COL_ANNOUNCE_ZERO 5

#define ROW_HUD_ZERO ROW_STAGE_ZERO
#define COL_HUD_ZERO COL_MAX + COL_STAGE_ZERO + 3

#define ROW_STAGE_NAME_ZERO 1
#define COL_STAGE_NAME_ZERO COL_STAGE_ZERO

#define ROW_DEBUG_ZERO 45
#define COL_DEBUG_ZERO 5

#define ALL_TERRAINS_SIZE 16
#define ALL_TERRAINS_FLOOR 0
#define ALL_TERRAINS_WALL_VERTICAL 1
#define ALL_TERRAINS_WALL_HORIZONTAL 2
#define ALL_TERRAINS_COLUMN 3

#define ALL_ACTORS_SIZE 32
#define ICON_PLAYER '@'
#define ICON_ITEM_DROP ';'

#define HUD_ROWS 24

#define ACTOR_INVENTORY_SIZE 64
#define ALL_ITEMS_SIZE ACTOR_INVENTORY_SIZE * ALL_ACTORS_SIZE * 2

#define BUTTON_QUIT 'q'

#define ANNOUNCEMENT_SIZE 128
#define DEBUG_MESSAGE_SIZE 128
#define STAGE_NAME_SIZE 128

#define DEBUG_PRINT_ENABLE 0

#if 0
move(ROW_DEBUG_ZERO + g_row_debug_current, COL_DEBUG_ZERO);
printw("");
g_row_debug_current++;
#endif

struct stats_combat {
	int hitpoints;
	int hitpoints_max;
};

struct actor {
	int row;
	int col;
	char icon;
	char name[32];
	int all_actors_index;
	void (*on_interact) (struct actor* const self, struct actor* const other);
	void (*despawn) (struct actor* const self);
	struct item* inventory[ACTOR_INVENTORY_SIZE];
	struct stats_combat combat;
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

enum cursor_movement {
	CURSOR_NONE,
	CURSOR_UP,
	CURSOR_DOWN,
};

enum hud_draw {
	DRAW_HIDE,
	DRAW_INVENTORY,
	DRAW_STATUS,
	DRAW_EQUIPMENT,
};

enum hud_toggle {
	TOGGLE_NONE,
	TOGGLE_INVENTORY,
	TOGGLE_STATUS,
	TOGGLE_EQUIPMENT,
};

enum position_update {
	POSITION_UPDATE_NONE,
	POSITION_UPDATE_UP,
	POSITION_UPDATE_DOWN,
	POSITION_UPDATE_LEFT,
	POSITION_UPDATE_RIGHT,
};

struct stage_shard g_stage[ROW_MAX][COL_MAX] = {0};
struct actor* g_all_actors[ALL_ACTORS_SIZE] = {0};

int g_all_actors_player_index;
int g_row_debug_current = 0;
enum hud_draw g_hud_to_draw = DRAW_HIDE;
int g_cursor_index = 0;
char g_stage_name[STAGE_NAME_SIZE] = {0};

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
		if (0 == actors[i]) {
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
			addch(g_stage[i][k].terrain->icon);
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

void draw_hud_inventory(struct item** inventory, const int cursor_pos)
{
	move(ROW_HUD_ZERO, COL_HUD_ZERO);
	printw("INVENTORY");
	for (int i = 0; i < HUD_ROWS; i++) {
		if (0 == inventory[i]) {
			break;
		}

		move(ROW_HUD_ZERO + 1 + i, COL_HUD_ZERO);
		if (cursor_pos == i) {
			printw("-> ");
		} else {
			printw("   ");
		}
		printw(inventory[i]->name);
	}
}

void draw_hud_status(struct actor* player)
{
	move(ROW_HUD_ZERO, COL_HUD_ZERO);
	printw("STATUS");
	move(ROW_HUD_ZERO + 2, COL_HUD_ZERO);
	printw("Name: %s", player->name);
	move(ROW_HUD_ZERO + 3, COL_HUD_ZERO);
	printw("Coordinates: (%d, %d)", player->row, player->col);
	move(ROW_HUD_ZERO + 4, COL_HUD_ZERO);
	printw("Hitpoints: %d", player->combat.hitpoints);
}

void draw_hud_equipment(void)
{
	move(ROW_HUD_ZERO, COL_HUD_ZERO);
	printw("EQUIPMENT");
	move(ROW_HUD_ZERO + 2, COL_HUD_ZERO);
	printw("Head:");
	move(ROW_HUD_ZERO + 3, COL_HUD_ZERO);
	printw("Chest:");
	move(ROW_HUD_ZERO + 4, COL_HUD_ZERO);
	printw("Right hand:");
	move(ROW_HUD_ZERO + 5, COL_HUD_ZERO);
	printw("Left hand:");
	move(ROW_HUD_ZERO + 6, COL_HUD_ZERO);
	printw("Legs:");
}

void draw_hud_stage_name(void)
{
	move(ROW_STAGE_NAME_ZERO, COL_STAGE_NAME_ZERO);
	printw(g_stage_name);
}

struct item** player_inventory(void)
{
	return g_all_actors[g_all_actors_player_index]->inventory;
}

void draw_layer_hud()
{
	switch(g_hud_to_draw) {
	case DRAW_HIDE:
		draw_hud_hide();
		break;
	case DRAW_INVENTORY:
		draw_hud_hide();
		draw_hud_inventory(player_inventory(), g_cursor_index);
		break;
	case DRAW_STATUS:
		draw_hud_hide();
		draw_hud_status(g_all_actors[g_all_actors_player_index]);
		break;
	case DRAW_EQUIPMENT:
		draw_hud_hide();
		draw_hud_equipment();
		break;
	}

	draw_hud_stage_name();
}

void draw(struct actor ** const all_actors)
{
	draw_layer_terrain();
	draw_layer_actors(all_actors);
	draw_layer_hud();
	refresh();
}

int is_direction_button(int* const pressed_key)
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

	return 0;
}

int is_traversable_terrain(int row, int col)
{
	return g_stage[row][col].terrain->traversable;
}

int is_occupied(int row, int col)
{
	return 0 != g_stage[row][col].occupant;
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

	self = g_stage[row][col].occupant;
	strcpy(str, initiator->name);
	strcat(str, " interacts with ");
	strcat(str, self->name);
	announce(str);
	g_stage[row][col].occupant->on_interact(self, initiator);
}

void update_position(
		enum position_update update,
		struct actor* const actor)
{
	int new_position_row = actor->row;
	int new_position_col = actor->col;

	if (POSITION_UPDATE_DOWN == update) {
		new_position_row = actor->row + 1;
	}

	if (POSITION_UPDATE_UP == update) {
		new_position_row = actor->row - 1;
	}

	if (POSITION_UPDATE_LEFT == update) {
		new_position_col = actor->col - 1;
	}

	if (POSITION_UPDATE_RIGHT == update) {
		new_position_col = actor->col + 1;
	}

	if (!is_traversable_terrain(new_position_row, new_position_col)) {
		return;
	}

	if (is_occupied(new_position_row, new_position_col)) {
		interact_with_occupant_of(new_position_row, new_position_col, actor);
		return;
	}

	g_stage[actor->row][actor->col].occupant = 0;
	actor->row = new_position_row;
	actor->col = new_position_col;
	g_stage[actor->row][actor->col].occupant = actor;
}

int is_hud_interactive(void) {
	return
		DRAW_INVENTORY == g_hud_to_draw;
}

int is_hud_selection_button(int* const pressed_key)
{
	if (is_hud_interactive() && '\n' == *pressed_key) {
		return 1;
	}

	return 0;
}

int is_cursor_movement_button(int* const pressed_key)
{
	if (is_hud_interactive() && is_direction_button(pressed_key)) {
		return 1;
	}

	return 0;
}

int is_hud_toggle_button(int* const pressed_key) {
	if ('i' == *pressed_key) {
		return 1;
	}

	if ('s' == *pressed_key) {
		return 1;
	}

	if ('e' == *pressed_key) {
		return 1;
	}

	return 0;
}

void toggle_hud_inventory(void)
{
	if (DRAW_INVENTORY == g_hud_to_draw) {
		g_hud_to_draw = DRAW_HIDE;
		return;
	}

	g_hud_to_draw = DRAW_INVENTORY;
	g_cursor_index = 0;
}

void toggle_hud_status(void)
{
	if (DRAW_STATUS == g_hud_to_draw) {
		g_hud_to_draw = DRAW_HIDE;
		return;
	}

	g_hud_to_draw = DRAW_STATUS;
}

void toggle_hud_equipment(void)
{
	if (DRAW_EQUIPMENT == g_hud_to_draw) {
		g_hud_to_draw = DRAW_HIDE;
		return;
	}

	g_hud_to_draw = DRAW_EQUIPMENT;
}

void toggle_hud(const enum hud_toggle toggle)
{
	if (TOGGLE_INVENTORY == toggle) {
		toggle_hud_inventory();
	}

	if (TOGGLE_STATUS == toggle) {
		toggle_hud_status();
	}

	if (TOGGLE_EQUIPMENT == toggle) {
		toggle_hud_equipment();
	}
}

void select_item(
		struct item** inventory,
		const int cursor)
{
	move(ROW_DEBUG_ZERO + g_row_debug_current, COL_DEBUG_ZERO);
	printw("Selected item %s", inventory[cursor]->name);
	g_row_debug_current++;
}

void move_cursor(const enum cursor_movement movement)
{
	int new_cursor;

	if (CURSOR_DOWN == movement) {
		new_cursor = g_cursor_index + 1;
	}

	if (CURSOR_UP == movement) {
		new_cursor = g_cursor_index - 1;
	}

	if (new_cursor < 0 || new_cursor > HUD_ROWS - 1) {
		// Out of bounds
		return;
	}

	g_cursor_index = new_cursor;
}

enum cursor_movement key_to_cursor_movement(int* const pressed_key)
{
	if (KEY_UP == *pressed_key)	{ return CURSOR_UP; }
	if (KEY_DOWN == *pressed_key)	{ return CURSOR_DOWN; }

	return CURSOR_NONE;
}

enum hud_toggle key_to_hud_toggle(int* const pressed_key)
{
	if ('i' == *pressed_key) { return TOGGLE_INVENTORY; }
	if ('s' == *pressed_key) { return TOGGLE_STATUS; }
	if ('e' == *pressed_key) { return TOGGLE_EQUIPMENT; }

	return TOGGLE_NONE;
}

enum position_update key_to_position_update(int* const pressed_key)
{
	if (KEY_UP == *pressed_key)	{ return POSITION_UPDATE_UP; }
	if (KEY_DOWN == *pressed_key)	{ return POSITION_UPDATE_DOWN; }
	if (KEY_LEFT == *pressed_key)	{ return POSITION_UPDATE_LEFT; }
	if (KEY_RIGHT == *pressed_key)	{ return POSITION_UPDATE_RIGHT; }

	return POSITION_UPDATE_NONE;
}

void update_hud(int* const pressed_key)
{
	if (is_hud_toggle_button(pressed_key)) {
		toggle_hud(key_to_hud_toggle(pressed_key));
		*pressed_key = 0;
	}

	if (is_cursor_movement_button(pressed_key)) {
		move_cursor(key_to_cursor_movement(pressed_key));
		*pressed_key = 0;
	}

	if (is_hud_selection_button(pressed_key)) {
		select_item(g_all_actors[g_all_actors_player_index]->inventory, g_cursor_index);
		*pressed_key = 0;
	}
}

void update_player(int* const pressed_key, struct actor* const player_actor)
{
	if (is_direction_button(pressed_key)) {
		update_position(key_to_position_update(pressed_key), player_actor);
		*pressed_key = 0;
	}
}

void update(
		int* const pressed_key,
		struct actor ** const all_actors)
{
	// Player and UI related updates dependent on user input.
	update_hud(pressed_key);

	update_player(pressed_key, all_actors[g_all_actors_player_index]);

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

void set_stage_name(char* new_stage_name)
{
	strcpy(g_stage_name, new_stage_name);
}

void set_stage_hideout(struct terrain ** const all_terrains)
{
	for (int i = 0; i < ROW_MAX; i++) {
		for (int k = 0; k < COL_MAX; k++) {
			if (is_corner(i, k)) {
				g_stage[i][k].terrain = all_terrains[ALL_TERRAINS_COLUMN];
			} else if (is_horizontal_edge(i, k)) {
				g_stage[i][k].terrain = all_terrains[ALL_TERRAINS_WALL_HORIZONTAL];
			} else if (is_vertical_edge(i, k)) {
				g_stage[i][k].terrain = all_terrains[ALL_TERRAINS_WALL_VERTICAL];
			} else {
				g_stage[i][k].terrain = all_terrains[ALL_TERRAINS_FLOOR];
			}
		}
	}

	set_stage_name("Hideout");
}

void load_stage(struct terrain ** const all_terrains)
{
	set_stage_hideout(all_terrains);
}

void add_to_inventory(struct item* item_to_add, struct item** inventory)
{
	int first_free = -1;
	char str[ANNOUNCEMENT_SIZE];
#if 0
	char str_debug[DEBUG_MESSAGE_SIZE];

	strcpy(str, "add_to_inventory() adding item: ");
	strcat(str, item_to_add->name);
	move(ROW_DEBUG_ZERO + g_row_debug_current, COL_DEBUG_ZERO);
	g_row_debug_current++;
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
	move(ROW_DEBUG_ZERO + g_row_debug_current, COL_DEBUG_ZERO);
	g_row_debug_current++;
	printw("%s", str);
#endif

	transfer_inventory_content(self->inventory, initiator->inventory);

	self->despawn(self);
}

void greet(struct actor* const self, struct actor* const initiator) {
	char str[ANNOUNCEMENT_SIZE] = {0};

	strcat(str, self->name);
	strcat(str, " greets ");
	strcat(str, initiator->name);
	announce(str);
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

void despawn_item_drop(struct actor* const self)
{
	g_stage[self->row][self->col].occupant = 0;
	g_all_actors[self->all_actors_index] = 0;
	free(self);
}

#if 0
void despawn_player()
{
}
#endif

void spawn_item_drop(
		const int row,
		const int col,
		struct actor ** const all_actors,
		struct item ** const all_items,
		const int quality)
{
	char str[ANNOUNCEMENT_SIZE] = {0};
	int ret = 0;
	int f = -1;
	int new_item_index = -1;

	f = get_first_free_actor_slot(all_actors);

	if (-1 == f) {
		strcpy(str, "Failed to spawn item drop, no free actor slots.");
		announce(str);
		return;
	}

	ret = spawn_item(all_items, quality, &new_item_index);
	if (0 != ret) {
		strcpy(str, "Failed to spawn item drop, no free item slots.");
		announce(str);
		return;
	}

	all_actors[f] = malloc(sizeof(struct actor));

	for (int i = 0; i < ACTOR_INVENTORY_SIZE; i++) {
		all_actors[f]->inventory[i] = 0;
	}
	strcpy(all_actors[f]->name, "item drop");
	all_actors[f]->row		= row;
	all_actors[f]->col		= col;
	all_actors[f]->icon		= ICON_ITEM_DROP;
	all_actors[f]->all_actors_index = f;
	all_actors[f]->on_interact	= get_picked;
	all_actors[f]->despawn		= despawn_item_drop;
	all_actors[f]->inventory[0]	= all_items[new_item_index];

	g_stage[row][col].occupant = all_actors[f];
}

void spawn_player(
		const int row,
		const int col,
		struct actor ** const all_actors)
{
	char str[ANNOUNCEMENT_SIZE] = {0};
	int f = -1;

	f = get_first_free_actor_slot(all_actors);

	if (-1 == f) {
		strcpy(str, "Failed to spawn player, no free actor slots.");
		announce(str);
		return;
	}

	all_actors[f] = malloc(sizeof(struct actor));

	for (int i = 0; i < ACTOR_INVENTORY_SIZE; i++) {
		all_actors[f]->inventory[i] = 0;
	}
	strcpy(all_actors[f]->name, "wizard");
	all_actors[f]->row		= row;
	all_actors[f]->col		= col;
	all_actors[f]->icon		= ICON_PLAYER;
	all_actors[f]->all_actors_index = f;
	all_actors[f]->on_interact	= greet;
#if 0
	all_actors[f]->despawn		= despawn_player;
#endif
	all_actors[f]->combat.hitpoints_max	= 100;
	all_actors[f]->combat.hitpoints		= all_actors[f]->combat.hitpoints_max;

	g_stage[row][col].occupant = all_actors[f];
	g_all_actors_player_index = f;
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

	srandom((unsigned) time(&t));

	initialize_io();
	load_stage(all_terrains);

	spawn_player(2, 2, g_all_actors);
	spawn_item_drop(4, 4, g_all_actors, all_items, 2);
	spawn_item_drop(5, 5, g_all_actors, all_items, 2);

	while (BUTTON_QUIT != pressed_key) {
		draw(g_all_actors);
		pressed_key = getch();
		update(&pressed_key, g_all_actors);
	}

	endwin();

	return 0;
}
