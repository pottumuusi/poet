#include "items_predefined.h"

#include "util_poet.h"
#include "log.h"

char g_map_environment_name[MAP_ENVIRONMENT_AMOUNT][MAP_NAME_LEN] = {
	"Dungeon",
	"Sewer",
};

static void apply_small_potion(struct actor* const target, struct item* const self);
static void map_on_consume(struct actor* const target, struct item* const self);
static void map_on_consume_actual(struct actor* const target, struct item* const self);

struct item* spawn_small_potion(void)
{
	return spawn_item_consumable("Small potion", apply_small_potion, 5);
}

static void apply_small_potion(struct actor* const target, struct item* const self)
{
	if (item_get_charges(self) > 0) {
		actor_hitpoints_increase(target, 20);
		item_charge_spend(self);
	}
}

struct item* spawn_item_map(int level, enum map_environment environment)
{
	struct item* map;
	char map_name[MAP_NAME_LEN];
	char level_num[2];

	snprintf(level_num, 2, "%d", level);
	strcpy(map_name, "Map lvl ");
	strcat(map_name, level_num);
	strcat(map_name, " ");
	strcat(map_name, g_map_environment_name[environment]);

	map = spawn_item_consumable(map_name, map_on_consume, 1);
	item_set_oneshot(map, 1);

	return map;
}

static void map_on_consume(struct actor* const target, struct item* const self)
{
	oneshot_on_consume(target, self, map_on_consume_actual);
}

static void map_on_consume_actual(struct actor* const target, struct item* const self)
{
	// Create portals
}
