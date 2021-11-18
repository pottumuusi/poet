#include "announce.h"
#include "interact.h"

void interact_with_occupant_of(int row, int col, struct actor* const initiator)
{
	char str[ANNOUNCEMENT_SIZE];
	struct actor* self = 0;

	self = g_stage[row][col].occupant;
	strcpy(str, initiator->name);
	strcat(str, " interacts with ");
	strcat(str, self->name);
	announce(str);
	g_stage[row][col].occupant->on_interact(self, initiator);
}

void get_picked(struct actor* const self, struct actor* const initiator)
{
#if 0
	char str[DEBUG_MESSAGE_SIZE];

	strcpy(str, "get_picked() called by: ");
	strcat(str, self->name);
	strcat(str, ", initiated by: ");
	strcat(str, initiator->name);
	move(ROW_DEBUG_ZERO + g_row_debug_current, COL_DEBUG_ZERO);
	g_row_debug_current++;
	printw("%s", str);
#endif

	transfer_inventory_content(self->inventory, initiator->inventory);

	self->despawn(self);
}

void greet(struct actor* const self, struct actor* const initiator)
{
	char str[ANNOUNCEMENT_SIZE] = {0};

	strcat(str, self->name);
	strcat(str, " greets ");
	strcat(str, initiator->name);
	announce(str);
}

void add_to_inventory(struct item* item_to_add, struct item** inventory)
{
	int first_free = -1;
	char str[ANNOUNCEMENT_SIZE];
#if 0
	char str_debug[DEBUG_MESSAGE_SIZE];

	strcpy(str, "add_to_inventory() adding item: ");
	strcat(str, item_to_add->name);
	move(ROW_DEBUG_ZERO + g_row_debug_current, COL_DEBUG_ZERO);
	g_row_debug_current++;
	printw("%s", str);
#endif

	first_free = get_first_free_inventory_slot(inventory);
	if (-1 == first_free) {
		strcpy(str, "Inventory full");
		announce(str);
		return;
	}

	inventory[first_free] = item_to_add;
}

void transfer_inventory_content(struct item** inventory_from, struct item** inventory_to)
{
	int i = 0;

	while (0 != inventory_from[i] && i < ACTOR_INVENTORY_SIZE) {
		add_to_inventory(inventory_from[i], inventory_to);
		inventory_from[0] = 0;
		i++;
	}
}
