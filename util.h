#ifndef UTIL_H_DEFINED
#define UTIL_H_DEFINED

int get_first_free_inventory_slot(struct item** const inventory);
int get_first_free_actor_slot(struct actor** const actors);
int get_first_free_item_slot(struct item** const items);

#endif
