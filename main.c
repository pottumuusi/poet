#include "actor.h"
#include "draw.h"
#include "item.h"
#include "stage.h"
#include "util.h"

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ROW_ANNOUNCE_ZERO 35
#define COL_ANNOUNCE_ZERO 5

#define ROW_DEBUG_ZERO 45
#define COL_DEBUG_ZERO 5

#define ALL_TERRAINS_SIZE 16

#define ICON_PLAYER '@'
#define ICON_ITEM_DROP ';'

#define ALL_ITEMS_SIZE ACTOR_INVENTORY_SIZE * ALL_ACTORS_SIZE * 2

#define BUTTON_QUIT 'q'

#define ANNOUNCEMENT_SIZE 128
#define DEBUG_MESSAGE_SIZE 128

#define DEBUG_PRINT_ENABLE 0

#if 0
move(ROW_DEBUG_ZERO + g_row_debug_current, COL_DEBUG_ZERO);
printw("");
g_row_debug_current++;
#endif

enum cursor_movement {
	CURSOR_NONE,
	CURSOR_UP,
	CURSOR_DOWN,
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

enum spawn_item_type {
	SPAWN_ITEM_TYPE_CONSUMABLE,
	SPAWN_ITEM_TYPE_EQUIPMENT,
	SPAWN_ITEM_TYPE_RANDOM,
};

int g_row_debug_current = 0;

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
	return DRAW_INVENTORY == g_hud_to_draw
		|| DRAW_SELECT_ITEM_OPERATION == g_hud_to_draw;
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
	bzero(g_hud_heading, HUD_HEADING_SIZE);
	strcpy(g_hud_heading, "INVENTORY");
	g_cursor_index = 0;
}

void toggle_hud_status(void)
{
	if (DRAW_STATUS == g_hud_to_draw) {
		g_hud_to_draw = DRAW_HIDE;
		return;
	}

	bzero(g_hud_heading, HUD_HEADING_SIZE);
	strcpy(g_hud_heading, "STATUS");
	g_hud_to_draw = DRAW_STATUS;
}

void toggle_hud_equipment(void)
{
	if (DRAW_EQUIPMENT == g_hud_to_draw) {
		g_hud_to_draw = DRAW_HIDE;
		return;
	}

	bzero(g_hud_heading, HUD_HEADING_SIZE);
	strcpy(g_hud_heading, "EQUIPMENT");
	g_hud_to_draw = DRAW_EQUIPMENT;
}

void set_hud_select_item_operation(
		struct item** inventory,
		const int cursor)
{
	bzero(g_hud_heading, HUD_HEADING_SIZE);
	strcpy(g_hud_heading, "For item - ");
	strcat(g_hud_heading, inventory[cursor]->name);
	strcat(g_hud_heading, ", do");
	g_hud_to_draw = DRAW_SELECT_ITEM_OPERATION;
	g_cursor_index = 0;
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
		if (DRAW_INVENTORY == g_hud_to_draw) {
			set_hud_select_item_operation(player_inventory(), g_cursor_index);
		}
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

int spawn_item(
		struct item ** const all_items,
		int quality,
		enum spawn_item_type type,
		int* new_item_index)
{
	int first_free = -1;

	first_free = get_first_free_item_slot(all_items);

	if (-1 == first_free) {
		return -1;
	}

	if (SPAWN_ITEM_TYPE_RANDOM == type) {
		if (0 == random() % 2) {
			spawn_item_consumable(all_items, first_free);
		} else {
			spawn_item_equipment(all_items, first_free);
		}
	} else if (SPAWN_ITEM_TYPE_CONSUMABLE == type) {
		spawn_item_consumable(all_items, first_free);
	} else if (SPAWN_ITEM_TYPE_EQUIPMENT == type) {
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
		const int quality,
		enum spawn_item_type type)
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

	ret = spawn_item(all_items, quality, type, &new_item_index);
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

	struct item_operation operation_use = {
		.name = "use"
	};

	struct item_operation operation_equip = {
		.name = "equip"
	};

	struct item_operation operation_drop = {
		.name = "drop"
	};

	all_terrains[ALL_TERRAINS_FLOOR]		= &floor;
	all_terrains[ALL_TERRAINS_WALL_VERTICAL]	= &wall_vertical;
	all_terrains[ALL_TERRAINS_WALL_HORIZONTAL]	= &wall_horizontal;
	all_terrains[ALL_TERRAINS_COLUMN]		= &column;

	g_item_operations[0] = &operation_use;
	g_item_operations[1] = &operation_equip;
	g_item_operations[2] = &operation_drop;

	srandom((unsigned) time(&t));

	initialize_io();
	load_stage(all_terrains);

	spawn_player(2, 2, g_all_actors);
	spawn_item_drop(4, 4, g_all_actors, all_items, 2, SPAWN_ITEM_TYPE_CONSUMABLE);
	spawn_item_drop(5, 5, g_all_actors, all_items, 2, SPAWN_ITEM_TYPE_EQUIPMENT);

	while (BUTTON_QUIT != pressed_key) {
		draw(g_all_actors);
		pressed_key = getch();
		update(&pressed_key, g_all_actors);
	}

	endwin();

	return 0;
}
