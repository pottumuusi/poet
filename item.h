#ifndef ITEM_H_DEFINED
#define ITEM_H_DEFINED

struct item {
	int amount;
	char consumable;
	char name[32];
	int all_items_index;
};

#endif
