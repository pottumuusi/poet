#include "actor.h"
#include "item.h"

int get_first_free_inventory_slot(struct item** const inventory)
{
	for (int i = 0; i < ACTOR_INVENTORY_SIZE; i++) {
		if (0 == inventory[i]) {
			return i;
		}
	}

	return -1;
}

int get_first_free_actor_slot(struct actor** const actors)
{
	for (int i = 0; i < ALL_ACTORS_SIZE; i++) {
		if (0 == actors[i]) {
			return i;
		}
	}

	return -1;
}

int get_first_free_item_slot(struct item** const items)
{
	return get_first_free_inventory_slot(items);
}
