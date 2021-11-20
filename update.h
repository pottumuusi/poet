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

static void update_position(
		enum position_update update,
		struct actor* const actor);
static void toggle_hud_inventory(void);
static void toggle_hud_status(void);
static void toggle_hud_equipment(void);
static void select_operation_for_item(struct item* const selected_item);
static void set_hud_select_item_operation(struct item* const selected_item);
static void set_hud_hide(void);
static void apply_operation_to_item(struct item* const selected_item, struct item_operation* operation);
static void toggle_hud(const enum hud_toggle toggle);
static void move_cursor(const enum cursor_movement movement);
static void update_hud(int* const pressed_key);
static void update_player(int* const pressed_key, struct actor* const player_actor);

#endif
