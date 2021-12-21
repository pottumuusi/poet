#include <stdlib.h>

#include "actor.h"
#include "log.h"
#include "util_poet.h"

struct actor* g_all_actors[ALL_ACTORS_SIZE] = {0};
struct actor* g_hostile_actors[ALL_ACTORS_SIZE] = {0};
int g_all_actors_player_index; // Initialized when player allocated

struct actor* get_player(void)
{
	struct actor* player = g_all_actors[g_all_actors_player_index];
	assert(0 != player);
	return player;
}

struct item** get_actor_inventory(struct actor* const a)
{
	assert(0 != a);
	return a->inventory;
}

struct item** get_actor_equipment(struct actor* const a)
{
	assert(0 != a);
	return a->equipment;
}

struct item* get_actor_item(struct actor* const a, int index)
{
	assert(0 != a);
	struct item** inventory = get_actor_inventory(a);
	struct item* item = inventory[index];
	assert(0 != item);
	return item;
}

char* get_actor_name(struct actor* const a)
{
	assert(0 != a);
	return a->name;
}

int actor_get_hitpoints(struct actor* const a)
{
	assert(0 != a);
	return a->combat.hitpoints;
}

struct actor** get_all_actors(void)
{
	return g_all_actors;
}

struct actor** get_all_hostile_actors(void)
{
	return g_hostile_actors;
}

int get_actor_row(struct actor* const a)
{
	assert(0 != a);
	return a->row;
}

int get_actor_col(struct actor* const a)
{
	assert(0 != a);
	return a->col;
}

int get_actor_is_hostile(struct actor* const a)
{
	assert(0 != a);
	return a->combat.is_hostile;
}

int actor_get_damage_unarmed(struct actor* const a)
{
	assert(0 != a);
	return a->combat.damage_unarmed;
}

int actor_get_damage_armed(struct actor* const a)
{
	assert(0 != a);
	return a->combat.damage_armed;
}

void set_actor_row(struct actor* const a, int new_row)
{
	assert(0 != a);

	if (!tile_is_occupied(new_row, get_actor_col(a))) {
		unoccupy_tile(get_actor_row(a), get_actor_col(a));
		a->row = new_row;
		occupy_tile(get_actor_row(a), get_actor_col(a), a);
	}
}

void set_actor_col(struct actor* const a, int new_col)
{
	assert(0 != a);
	if (!tile_is_occupied(get_actor_row(a), new_col)) {
		unoccupy_tile(get_actor_row(a), get_actor_col(a));
		a->col = new_col;
		occupy_tile(get_actor_row(a), get_actor_col(a), a);
	}
}

void actor_set_base_damage_unarmed(struct actor* const a, int base_damage)
{
	assert(0 != a);
	a->combat.base_damage_unarmed = base_damage;
}

void actor_set_base_damage_armed(struct actor* const a, int base_damage)
{
	assert(0 != a);
	a->combat.base_damage_armed = base_damage;
}

int actor_is_armed(struct actor* const a)
{
	int both_hands_empty;
	struct item** const equipment = get_actor_equipment(a);

	both_hands_empty =
		0 == equipment[EQUIPMENT_SLOT_RIGHT_HAND] &&
		0 == equipment[EQUIPMENT_SLOT_LEFT_HAND];
	if (both_hands_empty) {
		return 0;
	}

	return 1;
}

int player_has_spawned()
{
	return 0 != g_all_actors[g_all_actors_player_index];
}

void (*get_player_op_equip(void)) (struct item* const item_to_equip)
{
	struct actor* const player = get_player();
	return player->op_equip;
}

void actor_calculate_damage(struct actor* const a)
{
	a->combat.damage_unarmed = a->combat.base_damage_unarmed;
	a->combat.damage_armed = a->combat.base_damage_armed;
}

int actor_reduce_damage(struct actor* const a, unsigned int damage)
{
	return damage;
}

void actor_take_damage(struct actor* const a, unsigned int damage)
{
	struct actor* const p = get_player();

	a->combat.hitpoints = a->combat.hitpoints - damage;
	if (a->combat.hitpoints <= 0) {
		strcpy(g_new_announcement, a->name);
		strcat(g_new_announcement, " was vanquished");
		announce(g_new_announcement);

		/* TODO drop inventory */
		a->op_despawn(a);

		if (a == p) {
			game_over();
		}
	}
}

void spawn_item_consumable(struct item ** const all_items, int first_free)
{
	all_items[first_free] = malloc(sizeof(struct item));
	all_items[first_free]->consumable = 1;
	strcpy(all_items[first_free]->name, "potion");
	all_items[first_free]->all_items_index = first_free;
	all_items[first_free]->suitable_equipment_slot = EQUIPMENT_SLOT_NONE;
}

void spawn_item_equipment(
		struct item ** const all_items,
		int first_free,
		const char* name,
		int suitable_slot)
{
	all_items[first_free] = malloc(sizeof(struct item));
	all_items[first_free]->consumable = 0;
	strcpy(all_items[first_free]->name, name);
	all_items[first_free]->all_items_index = first_free;
	all_items[first_free]->suitable_equipment_slot = suitable_slot;
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
			spawn_item_equipment(
					all_items,
					first_free,
					"dagger",
					EQUIPMENT_SLOT_RIGHT_HAND);
		}
	} else if (SPAWN_ITEM_TYPE_CONSUMABLE == type) {
		spawn_item_consumable(all_items, first_free);
	} else if (SPAWN_ITEM_TYPE_EQUIPMENT == type) {
		spawn_item_equipment(
				all_items,
				first_free,
				"dagger",
				EQUIPMENT_SLOT_RIGHT_HAND);
	}

	*new_item_index = first_free;

	return 0;
}

void despawn_actor(struct actor* const self)
{
	struct actor** hostile_actors;
	int row = get_actor_row(self);
	int col = get_actor_col(self);

	g_stage[row][col].occupant = 0;
	g_all_actors[self->all_actors_index] = 0;

	if (self->combat.is_hostile) {
		hostile_actors = get_all_hostile_actors();
		hostile_actors[self->all_hostile_actors_index] = 0;
	}

	free(self);
}

void despawn_actor_player(struct actor* const self)
{
	int row = get_actor_row(self);
	int col = get_actor_col(self);

	g_stage[row][col].occupant = 0;
	g_all_actors[self->all_actors_index] = 0;
	free(self);
}

void despawn_all_actors(void)
{
	for (int i = 0; i < ALL_ACTORS_SIZE; i++) {
		if (0 != g_all_actors[i]) {
			g_all_actors[i]->op_despawn(g_all_actors[i]);
		}
	}
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
	int ret = 0;
	int f = -1;
	int new_item_index = -1;

	f = get_first_free_actor_slot(all_actors);

	if (-1 == f) {
		strcpy(g_new_announcement, "Failed to spawn item drop, no free actor slots.");
		announce(g_new_announcement);
		return;
	}

	ret = spawn_item(all_items, quality, type, &new_item_index);
	if (0 != ret) {
		strcpy(g_new_announcement, "Failed to spawn item drop, no free item slots.");
		announce(g_new_announcement);
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
	all_actors[f]->op_on_interact	= get_picked;
	all_actors[f]->op_despawn	= despawn_actor;
	all_actors[f]->op_equip		= 0; /* Only valid for player */
	all_actors[f]->inventory[0]	= all_items[new_item_index];

	g_stage[row][col].occupant = all_actors[f];

	LOG_INFO("Spawn %s at (%d %d)\n", all_actors[f]->name, row, col);
}

struct actor* spawn_actor(
		const char* name,
		const int row,
		const int col,
		const char icon,
		void (*despawn) (struct actor* const self),
		void (*on_interact) (struct actor* const self, struct actor* const other),
		const int hitpoints_max,
		const int is_hostile)
{
	int f = -1;
	int hf = -1;
	struct actor** const all_actors = get_all_actors();
	struct actor** const hostile_actors = get_all_hostile_actors();

	f = get_first_free_actor_slot(all_actors);
	if (-1 == f) {
		strcpy(g_new_announcement, "Failed to spawn ");
		strcat(g_new_announcement, name);
		strcat(g_new_announcement, ", no free actor slots.");
		announce(g_new_announcement);
		return 0;
	}

	all_actors[f] = malloc(sizeof(struct actor));
	if (is_hostile) {
		hf = get_first_free_actor_slot(hostile_actors);
		assert(-1 != hf);
		hostile_actors[hf] = all_actors[f];
	}

	for (int i = 0; i < ACTOR_INVENTORY_SIZE; i++) {
		all_actors[f]->inventory[i] = 0;
	}
	for (int i = 0; i < ACTOR_EQUIPMENT_SIZE; i++) {
		all_actors[f]->equipment[i] = 0;
	}

	strcpy(all_actors[f]->name, name);
	all_actors[f]->row				= row;
	all_actors[f]->col				= col;
	all_actors[f]->icon				= icon;
	all_actors[f]->all_actors_index 		= f;
	all_actors[f]->all_hostile_actors_index 	= hf;
	all_actors[f]->op_equip				= 0; /* Only valid for player */
	all_actors[f]->op_on_interact			= on_interact;
	all_actors[f]->op_despawn			= despawn;
	all_actors[f]->combat.hitpoints_max		= hitpoints_max;
	all_actors[f]->combat.hitpoints			= all_actors[f]->combat.hitpoints_max;
	all_actors[f]->combat.is_hostile		= is_hostile;
	all_actors[f]->combat.damage_unarmed		= 0;
	all_actors[f]->combat.damage_armed		= 0;
	all_actors[f]->combat.base_damage_unarmed	= 0;
	all_actors[f]->combat.base_damage_armed		= 0;
	all_actors[f]->attribute.dexterity		= 0;
	all_actors[f]->attribute.intelligence		= 0;
	all_actors[f]->attribute.strength		= 0;

	g_stage[row][col].occupant = all_actors[f];

	LOG_INFO("Spawn %s at (%d %d)\n", all_actors[f]->name, row, col);

	return all_actors[f];
}

void spawn_player(
		const int row,
		const int col,
		struct actor ** const all_actors)
{
	int f = -1;

	f = get_first_free_actor_slot(all_actors);

	if (-1 == f) {
		strcpy(g_new_announcement, "Failed to spawn player, no free actor slots.");
		announce(g_new_announcement);
		return;
	}

	all_actors[f] = malloc(sizeof(struct actor));

	for (int i = 0; i < ACTOR_INVENTORY_SIZE; i++) {
		all_actors[f]->inventory[i] = 0;
	}
	for (int i = 0; i < ACTOR_EQUIPMENT_SIZE; i++) {
		all_actors[f]->equipment[i] = 0;
	}
	strcpy(all_actors[f]->name, "wizard");
	all_actors[f]->row				= row;
	all_actors[f]->col				= col;
	all_actors[f]->icon				= ICON_PLAYER;
	all_actors[f]->all_actors_index 		= f;
	all_actors[f]->op_equip				= player_equip_item;
	all_actors[f]->op_on_interact			= do_combat;
	all_actors[f]->op_despawn			= despawn_actor_player;
	all_actors[f]->combat.hitpoints_max		= 100;
	all_actors[f]->combat.hitpoints			= all_actors[f]->combat.hitpoints_max;
	all_actors[f]->combat.is_hostile		= 1;
	all_actors[f]->combat.damage_unarmed		= 0;
	all_actors[f]->combat.damage_armed		= 0;
	all_actors[f]->combat.base_damage_unarmed	= 0;
	all_actors[f]->combat.base_damage_armed		= 0;
	all_actors[f]->attribute.dexterity		= 0;
	all_actors[f]->attribute.intelligence		= 0;
	all_actors[f]->attribute.strength		= 0;

	g_stage[row][col].occupant = all_actors[f];
	g_all_actors_player_index = f;

	actor_set_base_damage_unarmed(all_actors[f], 5);
	actor_calculate_damage(all_actors[f]);

	LOG_INFO("Spawn %s at (%d %d)\n", all_actors[f]->name, row, col);
}

void player_equip_item(struct item* const item_to_equip)
{
	struct actor* const player = get_player();
	int slot = EQUIPMENT_SLOT_NONE;

	slot = item_to_equip->suitable_equipment_slot;
	player->equipment[slot] = item_to_equip;

	strcpy(g_new_announcement, "Equipped item: ");
	strcat(g_new_announcement, player->equipment[slot]->name);
	announce(g_new_announcement);
}

struct actor* spawn_actor_skeleton(int row, int col)
{
	struct actor* a;

	a = spawn_actor(
			"skeleton",
			row, col,
			ICON_SKELETON,
			despawn_actor,
			do_combat,
			25,
			1);
	actor_set_base_damage_unarmed(a, 3);
	actor_calculate_damage(a);

	return a;
}
