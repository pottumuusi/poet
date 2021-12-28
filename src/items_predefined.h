#ifndef ITEMS_PREDEFINED_H_DEFINED
#define ITEMS_PREDEFINED_H_DEFINED

#define MAP_NAME_LEN 64
#define MAP_ENVIRONMENT_AMOUNT 2

enum map_environment {
	MAP_ENVIRONMENT_DUNGEON,
	MAP_ENVIRONMENT_SEWER,
};

extern char g_map_environment_name[MAP_ENVIRONMENT_AMOUNT][MAP_NAME_LEN];

struct item* spawn_small_potion(void);
struct item* spawn_item_map(int level, enum map_environment environment);

#endif
