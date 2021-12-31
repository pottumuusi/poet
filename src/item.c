#include <stdlib.h>
#include <string.h>

#include "item.h"
#include "items_predefined.h"
#include "log.h"
#include "util_poet.h"

struct item* g_selected_item = 0;
struct item* g_all_items[ALL_ITEMS_SIZE] = {0};
struct item_operation* g_item_operations[ITEM_OPERATIONS_SIZE] = {0};

static void item_charge_decrement(struct item* const t);
static void item_subcharge_increment(struct item* const t);
static void item_charge_increment(struct item* const t);
static void zero_item_fields(struct item* const item_to_zero);

struct item** get_all_items(void)
{
	return g_all_items;
}

struct item* get_selected_item(void)
{
	return g_selected_item;
}

void set_selected_item(struct item* new_item)
{
	g_selected_item = new_item;
}

struct item_operation* get_item_operation(int index)
{
	return g_item_operations[index];
}

void apply_operation_use(struct item* object_item)
{
	void (*op_use) (struct item* const item_to_use) = player_get_op_use();
	op_use(object_item);
}

void apply_operation_equip(struct item* object_item)
{
	void (*op_equip) (struct item* const item_to_equip) = player_get_op_equip();
	op_equip(object_item);
}

void apply_operation_drop(struct item* subject)
{
	LOG_DEBUG("%s\n", "apply_operation_drop()");
}

struct item* spawn_item_consumable(
		const char* name,
		void (*consume) (struct actor* const user, struct item* const self),
		const int charges_max)
{
	struct item* t;
	struct item** all_items;
	int first_free;

	t = malloc(sizeof(struct item));
	all_items = get_all_items();
	first_free = get_first_free_item_slot(all_items);

	if (-1 == first_free) {
		LOG_ERROR("%s", "No free item slots when spawning item");
		return 0;
	}

	zero_item_fields(t);
	all_items[first_free] = t;

	t->consumable = 1;
	strcpy(t->name, name);
	t->all_items_index = first_free;
	t->suitable_equipment_slot = EQUIPMENT_SLOT_NONE;
	t->consume = consume;
	t->charges_max = charges_max;
	t->charges = charges_max;
	t->subcharges_max = 10;

	return t;
}

struct item* spawn_item_equipment(
		struct item ** const all_items,
		int first_free,
		const char* name,
		int suitable_slot)
{
	all_items[first_free] = malloc(sizeof(struct item));
	all_items[first_free]->consumable = 0;
	strcpy(all_items[first_free]->name, name);
	all_items[first_free]->all_items_index = first_free;
	all_items[first_free]->suitable_equipment_slot = suitable_slot;

	return all_items[first_free];
}

static void zero_item_fields(struct item* const item_to_zero)
{
	item_to_zero->amount = 0;
	item_to_zero->consumable = 0;
	bzero(item_to_zero->name, 32);
	item_to_zero->all_items_index = 0;
	item_to_zero->suitable_equipment_slot = 0;
	item_to_zero->subcharges = 0;
	item_to_zero->subcharges_max = 0;
	item_to_zero->charges = 0;
	item_to_zero->charges_max = 0;
	item_to_zero->oneshot = 0;
	item_to_zero->has_been_used = 0;
	item_to_zero->inventory_index = 0;
	item_to_zero->consume = 0;
	item_to_zero->item_map = 0;
}

struct item* spawn_item(
		int quality,
		enum spawn_item_type type)
{
	struct item** const all_items = get_all_items();
	struct item* new_item = 0;
	int first_free = -1;

	first_free = get_first_free_item_slot(all_items);

	if (-1 == first_free) {
		LOG_ERROR("%s", "No free item slots when spawning item");
		return 0;
	}

	if (SPAWN_ITEM_TYPE_RANDOM == type) {
		if (0 == random() % 2) {
			new_item = spawn_small_potion();
		} else {
			new_item =  spawn_item_equipment(
					all_items,
					first_free,
					"dagger",
					EQUIPMENT_SLOT_RIGHT_HAND);
		}
	} else if (SPAWN_ITEM_TYPE_CONSUMABLE == type) {
		new_item = spawn_small_potion();
	} else if (SPAWN_ITEM_TYPE_EQUIPMENT == type) {
		new_item = spawn_item_equipment(
				all_items,
				first_free,
				"dagger",
				EQUIPMENT_SLOT_RIGHT_HAND);
	}

	return new_item;
}

void item_despawn(struct item* const item_to_despawn)
{
	struct item** all_items = get_all_items();

	all_items[item_to_despawn->all_items_index] = 0;
	free(item_to_despawn);
	// TODO free item_to_despawn->item_map, if exists
}

void item_add_to_inventory(struct item* item_to_add, struct item** inventory)
{
	int first_free = -1;
#if 0
	char str_debug[DEBUG_MESSAGE_SIZE];

	strcpy(str, "item_add_to_inventory() adding item: ");
	strcat(str, item_to_add->name);
	move(ROW_DEBUG_ZERO + g_row_debug_current, COL_DEBUG_ZERO);
	g_row_debug_current++;
	printw("%s", str);
#endif

	first_free = get_first_free_inventory_slot(inventory);
	if (-1 == first_free) {
		strcpy(g_new_announcement, "Inventory full");
		announce(g_new_announcement);
		return;
	}

	inventory[first_free] = item_to_add;
	item_set_inventory_index(item_to_add, first_free);
}

void transfer_inventory_content(struct item** inventory_from, struct item** inventory_to)
{
	int i = 0;

	while (0 != inventory_from[i] && i < ACTOR_INVENTORY_SIZE) {
		item_add_to_inventory(inventory_from[i], inventory_to);
		inventory_from[0] = 0;
		i++;
	}
}

void item_charge_spend(struct item* const t)
{
	if (!t->consumable) {
		return;
	}

	item_charge_decrement(t);
}

static void item_charge_decrement(struct item* const t)
{
	if (t->charges > 0) {
		t->charges--;
	}
}

void item_charge_refill(struct item* const t)
{
	if (!t->consumable) {
		return;
	}

	item_subcharge_increment(t);
}

static void item_subcharge_increment(struct item* const t)
{
	if (t->subcharges + 1 == t->subcharges_max) {
		item_charge_increment(t);
		t->subcharges = 0;
		return;
	}

	t->subcharges++;
}

static void item_charge_increment(struct item* const t)
{
	if (t->charges + 1 <= t->charges_max) {
		t->charges++;
	}
}

void oneshot_on_consume(
		struct actor* const target,
		struct item* const self,
		void (*received_on_consume) (struct actor* const target, struct item* const self))
{
	assert(1 == item_is_oneshot(self));

	if (item_has_been_used(self)) {
		return;
	}

	received_on_consume(target, self);

	item_set_has_been_used(self, 1);
}

int item_get_charges(struct item* const t)
{
	return t->charges;
}

int item_is_consumable(struct item* const t)
{
	return t->consumable;
}

char* item_get_name(struct item* const t)
{
	return t->name;
}

int item_is_oneshot(struct item* const t)
{
	return t->oneshot;
}

void item_set_oneshot(struct item* const t, int oneshot)
{
	t->oneshot = oneshot;
}

int item_has_been_used(struct item* const t)
{
	return t->has_been_used;
}

void item_set_has_been_used(struct item* const t, int has_been_used)
{
	t->has_been_used = has_been_used;
}

int item_get_inventory_index(struct item* const t)
{
	return t->inventory_index;
}

void item_set_inventory_index(struct item* const t, int index)
{
	t->inventory_index = index;
}

void item_set_item_map(struct item* const t, struct item_map* const item_map)
{
	t->item_map = item_map;
}
