#include "draw.h"

struct item_operation* g_item_operations[ITEM_OPERATIONS_SIZE] = {0};
enum hud_draw g_hud_to_draw = DRAW_HIDE;
char g_hud_heading[HUD_HEADING_SIZE] = {0};
int g_cursor_index = 0;

void draw(struct actor ** const all_actors)
{
	draw_layer_terrain();
	draw_layer_actors(all_actors);
	draw_layer_hud();
	refresh();
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

		printw(operations[i]->name);
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
		printw(inventory[i]->name);
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
	move(ROW_HUD_ZERO, COL_HUD_ZERO);
	printw("%s", g_hud_heading);
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
	case DRAW_SELECT_ITEM_OPERATION:
		draw_hud_hide();
		draw_hud_select_item_operation(g_item_operations, g_cursor_index);
		break;
	}

	draw_hud_stage_name();
}
