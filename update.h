#ifndef UPDATE_H_DEFINED
#define UPDATE_H_DEFINED

#include "actor.h"
#include "draw.h"
#include "item.h"
#include "stage.h"
#include "user_input.h"

void update(
		int* const pressed_key,
		struct actor ** const all_actors);

void update_position(
		enum position_update update,
		struct actor* const actor);
void toggle_hud_inventory(void);
void toggle_hud_status(void);
void toggle_hud_equipment(void);
void set_hud_select_item_operation(
		struct item** inventory,
		const int cursor);
void toggle_hud(const enum hud_toggle toggle);
void move_cursor(const enum cursor_movement movement);
void update_hud(int* const pressed_key);
void update_player(int* const pressed_key, struct actor* const player_actor);

#endif
