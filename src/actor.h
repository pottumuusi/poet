#ifndef ACTOR_H_DEFINED
#define ACTOR_H_DEFINED

#include "announce.h"
#include "interact.h"
#include "item.h"

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
	int is_hostile;
	unsigned int damage_unarmed;
	unsigned int damage_armed;
	unsigned int base_damage_unarmed;
	unsigned int base_damage_armed;
};

struct stats_attribute {
	unsigned int dexterity;
	unsigned int intelligence;
	unsigned int strength;
};

struct actor {
	int row;
	int col;
	char icon;
	char name[32];
	int all_actors_index;
	int all_hostile_actors_index;
	struct item* inventory[ACTOR_INVENTORY_SIZE];
	struct item* equipment[ACTOR_EQUIPMENT_SIZE];
	struct stats_combat combat;
	struct stats_attribute attribute;

	void (*op_despawn) (struct actor* const self);
	void (*op_equip) (struct item* const item_to_equip);
	void (*op_on_interact) (struct actor* const self, struct actor* const other);
};

extern struct actor* g_all_actors[ALL_ACTORS_SIZE];
extern struct actor* g_hostile_actors[ALL_ACTORS_SIZE];
extern int g_all_actors_player_index;

struct actor*	get_player(void);
struct actor**	get_all_actors(void);
struct actor**	get_all_hostile_actors(void);
struct item**	get_actor_inventory(struct actor* const a);
struct item**	get_actor_equipment(struct actor* const a);
struct item*	get_actor_item(struct actor* const a, int index);
int		get_actor_row(struct actor* const a);
int		get_actor_col(struct actor* const a);
int		get_actor_is_hostile(struct actor* const a);
char*		get_actor_name(struct actor* const a);
int		actor_get_hitpoints(struct actor* const a);
int		actor_get_damage_unarmed(struct actor* const a);
int		actor_get_damage_armed(struct actor* const a);
void		(*get_player_op_equip(void)) (struct item* const item_to_equip);
void		set_actor_row(struct actor* const a, int new_row);
void		set_actor_col(struct actor* const a, int new_col);
void		actor_set_base_damage_unarmed(struct actor* const a, int base_damage);
void		actor_set_base_damage_armed(struct actor* const a, int base_damage);
int		actor_is_armed(struct actor* const a);
int		player_has_spawned(void);
void		actor_calculate_damage(struct actor* const a);
int		actor_reduce_damage(struct actor* const a, unsigned int damage);
void		actor_take_damage(struct actor* const a, unsigned int damage);

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
void despawn_actor(struct actor* const self);
void despawn_all_actors(void);
void spawn_item_drop(
		const int row,
		const int col,
		struct actor ** const all_actors,
		struct item ** const all_items,
		const int quality,
		enum spawn_item_type type);
struct actor* spawn_actor(
		const char* name,
		const int row,
		const int col,
		const char icon,
		void (*despawn) (struct actor* const self),
		void (*on_interact) (struct actor* const self, struct actor* const other),
		const int hitpoints_max,
		const int is_hostile);
void spawn_player(
		const int row,
		const int col,
		struct actor ** const all_actors);
void player_equip_item(struct item* const item_to_equip);
struct actor* spawn_actor_skeleton(int row, int col);

#endif
