#ifndef ITEMS_PREDEFINED_H_DEFINED
#define ITEMS_PREDEFINED_H_DEFINED

#include "items_predefined.h.types"

extern char g_map_environment_name[MAP_ENVIRONMENT_AMOUNT][MAP_NAME_LEN];

struct item* spawn_small_potion(void);
struct item* spawn_item_map(int level, enum map_environment environment);

#endif
