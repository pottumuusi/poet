#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "actor.h"
#include "announce.h"
#include "draw.h"
#include "interact.h"
#include "item.h"
#include "stage.h"
#include "update.h"
#include "user_input.h"
#include "util.h"

#define ROW_DEBUG_ZERO 45
#define COL_DEBUG_ZERO 5

#define ALL_TERRAINS_SIZE 16

#define ALL_ITEMS_SIZE ACTOR_INVENTORY_SIZE * ALL_ACTORS_SIZE * 2

#define BUTTON_QUIT 'q'

#define DEBUG_MESSAGE_SIZE 128

#define DEBUG_PRINT_ENABLE 0

#if 0
move(ROW_DEBUG_ZERO + g_row_debug_current, COL_DEBUG_ZERO);
printw("");
g_row_debug_current++;
#endif

int g_row_debug_current = 0;

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
