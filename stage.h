#ifndef STAGE_H_DEFINED
#define STAGE_H_DEFINED

#include <string.h>

#define ROW_STAGE_ZERO 3
#define COL_STAGE_ZERO 5

#define ROW_STAGE_NAME_ZERO 1
#define COL_STAGE_NAME_ZERO COL_STAGE_ZERO

#define ROW_STAGE_MAX 15
#define COL_STAGE_MAX 25

#define STAGE_NAME_SIZE 128

#define ALL_TERRAINS_FLOOR 0
#define ALL_TERRAINS_WALL_VERTICAL 1
#define ALL_TERRAINS_WALL_HORIZONTAL 2
#define ALL_TERRAINS_COLUMN 3

struct terrain {
	char icon;
	const char name[32];
	char traversable;
};

struct stage_shard {
	struct terrain* terrain;
	struct actor* occupant;
};

void load_stage(struct terrain ** const all_terrains);
int is_traversable_terrain(int row, int col);
int is_occupied(int row, int col);

static void set_stage_name(char* new_stage_name);
static int is_corner(int i, int k);
static int is_horizontal_edge(int i, int k);
static int is_vertical_edge(int i, int k);
static void set_stage_hideout(struct terrain ** const all_terrains);

extern struct stage_shard g_stage[ROW_STAGE_MAX][COL_STAGE_MAX];
extern char g_stage_name[STAGE_NAME_SIZE];

#endif
