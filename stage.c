#include "actor.h"
#include "log.h"
#include "stage.h"

struct terrain* g_all_terrains[ALL_TERRAINS_SIZE] = {0};
struct stage_shard g_stage_slice[ROW_DRAW_STAGE_LEN][COL_DRAW_STAGE_LEN];
struct stage_shard g_stage[STAGE_SIZE_VERTICAL][STAGE_SIZE_HORIZONTAL] = {0};
char g_stage_name[STAGE_NAME_SIZE] = {0};

void load_stage(enum stage_type s_type, struct terrain ** const all_terrains)
{
	if (STAGE_TYPE_HIDEOUT == s_type) {
		set_stage_hideout(all_terrains);
	} else if (STAGE_TYPE_DUNGEON) {
		generate_stage_dungeon(all_terrains);
	}
}

int is_traversable_terrain(int row, int col)
{
	return g_stage[row][col].terrain->traversable;
}

int is_occupied(int row, int col)
{
	return 0 != g_stage[row][col].occupant;
}

/*
 * Select shards from full stage to a stage slice.
 */
void set_stage_slice_around_player(void)
{
	int cursor_horizontal = 0;
	int cursor_vertical = 0;
	int min_horizontal = 0;
	int max_horizontal = 0;
	int min_vertical = 0;
	int max_vertical = 0;
	int p_row = 0;
	int p_col = 0;

#if 0
	LOG_DEBUG("p_row is: %d\n", p_row);
	LOG_DEBUG("p_col is: %d\n", p_col);
	LOG_DEBUG("ROW_DRAW_STAGE_LEN is: %d\n", ROW_DRAW_STAGE_LEN);
	LOG_DEBUG("COL_DRAW_STAGE_LEN is: %d\n", COL_DRAW_STAGE_LEN);
	LOG_DEBUG("ROW_DRAW_STAGE_LEN / 2 is: %d\n", ROW_DRAW_STAGE_LEN / 2);
	LOG_DEBUG("COL_DRAW_STAGE_LEN / 2 is: %d\n", COL_DRAW_STAGE_LEN / 2);
#endif

	p_row = get_player_row();
	p_col = get_player_col();
	min_vertical	= p_row - (ROW_DRAW_STAGE_LEN / 2);
	max_vertical	= p_row + (ROW_DRAW_STAGE_LEN / 2);
	min_horizontal	= p_col - (COL_DRAW_STAGE_LEN / 2);
	max_horizontal	= p_col + (COL_DRAW_STAGE_LEN / 2);
#if 0
	LOG_DEBUG("Drawing with limits: h_min %d, h_max %d, v_min %d, v_max %d\n",
			min_horizontal, max_horizontal, min_vertical, max_vertical);
#endif

	cursor_horizontal = min_horizontal;
	cursor_vertical = min_vertical;

	for (int i = 0; i < ROW_DRAW_STAGE_LEN; i++) {
		for (int k = 0; k < COL_DRAW_STAGE_LEN; k++) {
			add_shard_to_slice(i, k, cursor_vertical, cursor_horizontal);
			cursor_horizontal++;
		}

		cursor_horizontal = min_horizontal;
		cursor_vertical++;
	}
}

static void add_shard_to_slice(
		int row,
		int col,
		const int cursor_vertical,
		const int cursor_horizontal)
{
	// Example values to receive:
	// h_min -3, h_max 7, v_min -1, v_max 5

	if (cursor_vertical < 0) {
		g_stage_slice[row][col].terrain = g_all_terrains[ALL_TERRAINS_VOID];
		return;
	}

	if (cursor_vertical > STAGE_SIZE_VERTICAL - 1) {
		g_stage_slice[row][col].terrain = g_all_terrains[ALL_TERRAINS_VOID];
		return;
	}

	if (cursor_horizontal < 0) {
		g_stage_slice[row][col].terrain = g_all_terrains[ALL_TERRAINS_VOID];
		return;
	}

	if (cursor_horizontal > STAGE_SIZE_HORIZONTAL - 1) {
		g_stage_slice[row][col].terrain = g_all_terrains[ALL_TERRAINS_VOID];
		return;
	}

	g_stage_slice[row][col] = g_stage[cursor_vertical][cursor_horizontal];

	assert(0 != g_stage_slice[row][col].terrain);
}

static void set_stage_name(char* new_stage_name)
{
	strcpy(g_stage_name, new_stage_name);
}

static int is_corner(int i, int k)
{
	if (0 == i && 0 == k) {
		return 1;
	}

	if (STAGE_SIZE_VERTICAL - 1 == i && 0 == k) {
		return 1;
	}

	if (0 == i && STAGE_SIZE_HORIZONTAL - 1 == k) {
		return 1;
	}

	if (STAGE_SIZE_VERTICAL - 1 == i && STAGE_SIZE_HORIZONTAL - 1 == k) {
		return 1;
	}

	return 0;
}

static int is_horizontal_edge(int i, int k)
{
	if (0 == i) {
		return 1;
	}

	if (STAGE_SIZE_VERTICAL - 1 == i) {
		return 1;
	}

	return 0;
}

static int is_vertical_edge(int i, int k)
{
	if (0 == k) {
		return 1;
	}

	if (STAGE_SIZE_HORIZONTAL - 1 == k) {
		return 1;
	}

	return 0;
}

static void set_stage_hideout(struct terrain ** const all_terrains)
{
	for (int i = 0; i < STAGE_SIZE_VERTICAL; i++) {
		for (int k = 0; k < STAGE_SIZE_HORIZONTAL; k++) {
			if (is_corner(i, k)) {
				g_stage[i][k].terrain = all_terrains[ALL_TERRAINS_COLUMN];
			} else if (is_horizontal_edge(i, k)) {
				g_stage[i][k].terrain = all_terrains[ALL_TERRAINS_WALL_HORIZONTAL];
			} else if (is_vertical_edge(i, k)) {
				g_stage[i][k].terrain = all_terrains[ALL_TERRAINS_WALL_VERTICAL];
			} else {
				g_stage[i][k].terrain = all_terrains[ALL_TERRAINS_FLOOR];
			}
		}
	}

	set_stage_name("Hideout");
}

static void generate_stage_dungeon(struct terrain ** const all_terrains)
{
	set_stage_name("Dungeon");
}
