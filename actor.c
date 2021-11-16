#include "actor.h"

struct actor* g_all_actors[ALL_ACTORS_SIZE] = {0};
int g_all_actors_player_index; // Initialized when player allocated

struct item** player_inventory(void)
{
	return g_all_actors[g_all_actors_player_index]->inventory;
}
