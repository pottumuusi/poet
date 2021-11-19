#ifndef ACTOR_H_DEFINED
#define ACTOR_H_DEFINED

#include "announce.h"
#include "interact.h"
#include "item.h"

#define ALL_ACTORS_SIZE 32
#define ACTOR_INVENTORY_SIZE 64
#define ACTOR_EQUIPMENT_SIZE 6

#define ICON_PLAYER '@'
#define ICON_ITEM_DROP ';'

enum equipment_slot {
	EQUIPMENT_SLOT_NONE,
	EQUIPMENT_SLOT_HEAD,
	EQUIPMENT_SLOT_CHEST,
	EQUIPMENT_SLOT_RIGHT_HAND,
	EQUIPMENT_SLOT_LEFT_HAND,
	EQUIPMENT_SLOT_LEGS,
};

enum spawn_item_type {
	SPAWN_ITEM_TYPE_CONSUMABLE,
	SPAWN_ITEM_TYPE_EQUIPMENT,
	SPAWN_ITEM_TYPE_RANDOM,
};

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
	struct item* inventory[ACTOR_INVENTORY_SIZE];
	struct item* equipment[ACTOR_EQUIPMENT_SIZE];
	struct stats_combat combat;

	void (*despawn) (struct actor* const self);
	void (*equip) (struct item* const item_to_equip);
	void (*on_interact) (struct actor* const self, struct actor* const other);
};

extern struct actor* g_all_actors[ALL_ACTORS_SIZE];
extern int g_all_actors_player_index;

struct actor* get_player(void);
struct item** get_player_inventory(void);
struct item* get_player_item(int index);
void spawn_item_consumable(struct item ** const all_items, int first_free);
void spawn_item_equipment(
		struct item ** const all_items,
		int first_free,
		const char* name,
		int suitable_slot);
int spawn_item(
		struct item ** const all_items,
		int quality,
		enum spawn_item_type type,
		int* new_item_index);
void despawn_item_drop(struct actor* const self);
void spawn_item_drop(
		const int row,
		const int col,
		struct actor ** const all_actors,
		struct item ** const all_items,
		const int quality,
		enum spawn_item_type type);
void spawn_player(
		const int row,
		const int col,
		struct actor ** const all_actors);
void player_equip_item(struct item* const item_to_equip);

#endif
