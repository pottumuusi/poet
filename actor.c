#include <stdlib.h>

#include "actor.h"
#include "log.h"

struct actor* g_all_actors[ALL_ACTORS_SIZE] = {0};
int g_all_actors_player_index; // Initialized when player allocated

struct item** player_inventory(void)
{
	return g_all_actors[g_all_actors_player_index]->inventory;
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

	LOG_INFO("Spawn %s at (%d %d)\n", all_actors[f]->name, row, col);
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

	LOG_INFO("Spawn %s at (%d %d)\n", all_actors[f]->name, row, col);
}
