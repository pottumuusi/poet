#include "actor.h"
#include "announce.h"
#include "interact.h"
#include "item.h"
#include "log.h"
#include "update.h"

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

void set_hud_hide(void)
{
	g_hud_to_draw = DRAW_HIDE;
}

void set_hud_select_item_operation(struct item* const selected_item)
{
	bzero(g_hud_heading, HUD_HEADING_SIZE);
	strcpy(g_hud_heading, "For item - ");
	strcat(g_hud_heading, selected_item->name);
	strcat(g_hud_heading, ", do");
	g_hud_to_draw = DRAW_SELECT_ITEM_OPERATION;
	g_cursor_index = 0;
}

void select_operation_for_item(struct item* const selected_item)
{
	set_selected_item(selected_item);
	set_hud_select_item_operation(selected_item);
}

void apply_operation_to_item(struct item* const selected_item, struct item_operation* operation)
{
	LOG_INFO("apply operation %s for item %s\n", operation->name, selected_item->name);
	operation->apply(selected_item);
	set_hud_hide();
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

void update_hud(int* const pressed_key)
{
	if (is_hud_toggle_button(pressed_key)) {
		toggle_hud(key_to_hud_toggle(pressed_key));
		*pressed_key = 0;
		return;
	}

	if (is_cursor_movement_button(pressed_key)) {
		move_cursor(key_to_cursor_movement(pressed_key));
		*pressed_key = 0;
		return;
	}

	if (is_hud_select_button(pressed_key)) {
		if (DRAW_INVENTORY == g_hud_to_draw) {
			select_operation_for_item(get_player_item(g_cursor_index));
			*pressed_key = 0;
			return;
		}
		if (DRAW_SELECT_ITEM_OPERATION == g_hud_to_draw) {
			apply_operation_to_item(get_selected_item(), get_item_operation(g_cursor_index));
			*pressed_key = 0;
			return;
		}
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
