#include "draw.h"
#include "log.h"

struct item_operation* g_item_operations[ITEM_OPERATIONS_SIZE] = {0};
enum hud_draw g_hud_to_draw = DRAW_HIDE;
char g_hud_heading[HUD_HEADING_SIZE] = {0};
int g_cursor_index = 0;

void draw(struct actor ** const all_actors)
{
	set_stage_slice_around_player();
	draw_stage();
	draw_hud();
	refresh();
}

void draw_stage(void)
{
	const int sight = 4;

	move(ROW_STAGE_ZERO, COL_STAGE_ZERO);
	for (int i = 0; i < ROW_STAGE_LEN; i++) {
		for (int k = 0; k < COL_STAGE_LEN; k++) {
			undraw_stage_shard(i, k);
			draw_stage_shard(i, k);
#if 0
			LOG_DEBUG("(%d, %d) ", i, k);
#endif
		}
#if 0
		LOG_DEBUG("%s", "\n");
#endif
	}
}

static void undraw_stage_shard(int row, int col)
{
	move(ROW_STAGE_ZERO + row, COL_STAGE_ZERO + col);
	addch(' ');
}

static void draw_stage_shard(int row, int col)
{
	move(ROW_STAGE_ZERO + row, COL_STAGE_ZERO + col);
	addch(g_stage_slice[row][col].terrain->icon);

	if (0 != g_stage_slice[row][col].occupant) {
		move(ROW_STAGE_ZERO + row, COL_STAGE_ZERO + col);
		addch(g_stage_slice[row][col].occupant->icon);
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
		printw("%s", space_row);
	}
}

void draw_hud_select_item_operation(struct item_operation** operations, const int cursor_pos)
{
	move(ROW_HUD_ZERO, COL_HUD_ZERO);
	printw("%s", g_hud_heading);
	for (int i = 0; i < ITEM_OPERATIONS_SIZE; i++) {
		if (0 == operations[i]) {
			break;
		}

		move(ROW_HUD_ZERO + 1 + i, COL_HUD_ZERO);
		if (cursor_pos == i) {
			printw("-> ");
		} else {
			printw("   ");
		}

		printw("%s", operations[i]->name);
	}
}

void draw_hud_inventory(struct item** inventory, const int cursor_pos)
{
	move(ROW_HUD_ZERO, COL_HUD_ZERO);
	printw("%s", g_hud_heading);
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
		printw("%s", inventory[i]->name);
	}
}

void draw_hud_status(struct actor* player)
{
	move(ROW_HUD_ZERO, COL_HUD_ZERO);
	printw("%s", g_hud_heading);
	move(ROW_HUD_ZERO + 2, COL_HUD_ZERO);
	printw("Name: %s", player->name);
	move(ROW_HUD_ZERO + 3, COL_HUD_ZERO);
	printw("Coordinates: (%d, %d)", player->row, player->col);
	move(ROW_HUD_ZERO + 4, COL_HUD_ZERO);
	printw("Hitpoints: %d", player->combat.hitpoints);
}

void draw_hud_equipment(void)
{
	struct item** const equipment = get_player_equipment();

	move(ROW_HUD_ZERO, COL_HUD_ZERO);
	printw("%s", g_hud_heading);

	move(ROW_HUD_ZERO + 2, COL_HUD_ZERO);
	printw("Head: ");
	if (0 != equipment[EQUIPMENT_SLOT_HEAD]) {
		printw("%s", equipment[EQUIPMENT_SLOT_HEAD]->name);
	}

	move(ROW_HUD_ZERO + 3, COL_HUD_ZERO);
	printw("Chest: ");
	if (0 != equipment[EQUIPMENT_SLOT_CHEST]) {
		printw("%s", equipment[EQUIPMENT_SLOT_CHEST]->name);
	}

	move(ROW_HUD_ZERO + 4, COL_HUD_ZERO);
	printw("Right hand: ");
	if (0 != equipment[EQUIPMENT_SLOT_RIGHT_HAND]) {
		printw("%s", equipment[EQUIPMENT_SLOT_RIGHT_HAND]->name);
	}

	move(ROW_HUD_ZERO + 5, COL_HUD_ZERO);
	printw("Left hand: ");
	if (0 != equipment[EQUIPMENT_SLOT_LEFT_HAND]) {
		printw("%s", equipment[EQUIPMENT_SLOT_LEFT_HAND]->name);
	}

	move(ROW_HUD_ZERO + 6, COL_HUD_ZERO);
	printw("Legs: ");
	if (0 != equipment[EQUIPMENT_SLOT_LEGS]) {
		printw("%s", equipment[EQUIPMENT_SLOT_LEGS]->name);
	}
}

void draw_hud_stage_name(void)
{
	move(ROW_STAGE_NAME_ZERO, COL_STAGE_NAME_ZERO);
	printw("%s", g_stage_name);
}

void draw_hud(void)
{
	switch(g_hud_to_draw) {
	case DRAW_HIDE:
		draw_hud_hide();
		break;
	case DRAW_INVENTORY:
		draw_hud_hide();
		draw_hud_inventory(get_player_inventory(), g_cursor_index);
		break;
	case DRAW_STATUS:
		draw_hud_hide();
		draw_hud_status(g_all_actors[g_all_actors_player_index]);
		break;
	case DRAW_EQUIPMENT:
		draw_hud_hide();
		draw_hud_equipment();
		break;
	case DRAW_SELECT_ITEM_OPERATION:
		draw_hud_hide();
		draw_hud_select_item_operation(g_item_operations, g_cursor_index);
		break;
	}

	draw_hud_stage_name();
}
