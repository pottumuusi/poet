#include "stage.h"

struct stage_shard g_stage[ROW_STAGE_MAX][COL_STAGE_MAX] = {0};
char g_stage_name[STAGE_NAME_SIZE] = {0};

void load_stage(struct terrain ** const all_terrains)
{
	set_stage_hideout(all_terrains);
}

int is_traversable_terrain(int row, int col)
{
	return g_stage[row][col].terrain->traversable;
}

int is_occupied(int row, int col)
{
	return 0 != g_stage[row][col].occupant;
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

	if (ROW_STAGE_MAX - 1 == i && 0 == k) {
		return 1;
	}

	if (0 == i && COL_STAGE_MAX - 1 == k) {
		return 1;
	}

	if (ROW_STAGE_MAX - 1 == i && COL_STAGE_MAX - 1 == k) {
		return 1;
	}

	return 0;
}

static int is_horizontal_edge(int i, int k)
{
	if (0 == i) {
		return 1;
	}

	if (ROW_STAGE_MAX - 1 == i) {
		return 1;
	}

	return 0;
}

static int is_vertical_edge(int i, int k)
{
	if (0 == k) {
		return 1;
	}

	if (COL_STAGE_MAX - 1 == k) {
		return 1;
	}

	return 0;
}

static void set_stage_hideout(struct terrain ** const all_terrains)
{
	for (int i = 0; i < ROW_STAGE_MAX; i++) {
		for (int k = 0; k < COL_STAGE_MAX; k++) {
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
