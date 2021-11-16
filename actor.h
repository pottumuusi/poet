#ifndef ACTOR_H_DEFINED
#define ACTOR_H_DEFINED

#define ALL_ACTORS_SIZE 32
#define ACTOR_INVENTORY_SIZE 64

struct stats_combat {
	int hitpoints;
	int hitpoints_max;
};

struct actor {
	int row;
	int col;
	char icon;
	char name[32];
	int all_actors_index;
	void (*on_interact) (struct actor* const self, struct actor* const other);
	void (*despawn) (struct actor* const self);
	struct item* inventory[ACTOR_INVENTORY_SIZE];
	struct stats_combat combat;
};

struct item** player_inventory(void);

extern struct actor* g_all_actors[ALL_ACTORS_SIZE];
extern int g_all_actors_player_index;

#endif
