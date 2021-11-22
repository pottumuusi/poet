#ifndef STAGE_H_DEFINED
#define STAGE_H_DEFINED

#include <string.h>

#define ROW_STAGE_NAME_ZERO 1
#define COL_STAGE_NAME_ZERO COL_STAGE_ZERO

// TODO
// * move to draw.h
// * rename: ROW_STAGE_ZERO -> ROW_DRAW_STAGE_ZERO
#define ROW_STAGE_ZERO 3
#define COL_STAGE_ZERO 5
#define ROW_STAGE_MAX 15
#define COL_STAGE_MAX 25
#define ROW_STAGE_LEN (ROW_STAGE_MAX - ROW_STAGE_ZERO)
#define COL_STAGE_LEN (COL_STAGE_MAX - COL_STAGE_ZERO)

#define STAGE_SIZE_HORIZONTAL 20
#define STAGE_SIZE_VERTICAL STAGE_SIZE_HORIZONTAL

#define STAGE_NAME_SIZE 128

#define ALL_TERRAINS_FLOOR 0
#define ALL_TERRAINS_WALL_VERTICAL 1
#define ALL_TERRAINS_WALL_HORIZONTAL 2
#define ALL_TERRAINS_COLUMN 3
#define ALL_TERRAINS_VOID 4

#define ALL_TERRAINS_SIZE 16

enum stage_type {
	STAGE_TYPE_HIDEOUT,
	STAGE_TYPE_DUNGEON,
};

struct terrain {
	char icon;
	const char name[32];
	char traversable;
};

// TODO rename to tile
struct stage_shard {
	struct terrain* terrain;
	struct actor* occupant;
};

void load_stage(enum stage_type s_type, struct terrain ** const all_terrains);
int is_traversable_terrain(int row, int col);
int is_occupied(int row, int col);
void set_stage_slice_around_player(void);

static void add_shard_to_slice(
		int row,
		int col,
		const int cursor_vertical,
		const int cursor_horizontal);
static void set_stage_name(char* new_stage_name);
static int is_corner(int i, int k);
static int is_horizontal_edge(int i, int k);
static int is_vertical_edge(int i, int k);
static void set_stage_hideout(struct terrain ** const all_terrains);
static void generate_stage_dungeon(struct terrain ** const all_terrains);

extern struct terrain* g_all_terrains[ALL_TERRAINS_SIZE];
extern struct stage_shard g_stage_slice[ROW_STAGE_LEN][COL_STAGE_LEN];
extern struct stage_shard g_stage[STAGE_SIZE_VERTICAL][STAGE_SIZE_HORIZONTAL];
extern char g_stage_name[STAGE_NAME_SIZE];

#endif
