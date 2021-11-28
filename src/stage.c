#include "actor.h"
#include "item.h"
#include "log.h"
#include "stage.h"

struct terrain* g_all_terrains[ALL_TERRAINS_SIZE] = {0};
struct tile g_stage_slice[ROW_DRAW_STAGE_LEN][COL_DRAW_STAGE_LEN] = {0};
struct tile g_stage[STAGE_SIZE_VERTICAL][STAGE_SIZE_HORIZONTAL] = {0};
char g_stage_name[STAGE_NAME_SIZE] = {0};

static void add_tile_to_slice(
		int row,
		int col,
		const int cursor_vertical,
		const int cursor_horizontal);
static void set_stage_name(char* new_stage_name);
static int is_corner(int i, int k);
static int is_horizontal_edge(int i, int k);
static int is_vertical_edge(int i, int k);
static void load_stage_hideout(void);
static void load_stage_dungeon(void);

void load_stage(enum stage_type s_type)
{
	if (STAGE_TYPE_HIDEOUT == s_type) {
		load_stage_hideout();
	} else if (STAGE_TYPE_DUNGEON) {
		load_stage_dungeon();
	}
}

int tile_is_traversable_terrain(int row, int col)
{
	return g_stage[row][col].terrain->traversable;
}

int tile_is_occupied(int row, int col)
{
	return 0 != g_stage[row][col].occupant;
}

void occupy_tile(int row, int col, struct actor* a)
{
	g_stage[row][col].occupant = a;
}

void unoccupy_tile(int row, int col)
{
	g_stage[row][col].occupant = 0;
}

/*
 * Select tiles from full stage to a stage slice.
 */
void set_stage_slice_around_player(void)
{
	int cursor_horizontal = 0;
	int cursor_vertical = 0;
	int min_horizontal = 0;
	int min_vertical = 0;
#if 0
	int max_horizontal = 0;
	int max_vertical = 0;
#endif
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

	p_row = get_actor_row(get_player());
	p_col = get_actor_col(get_player());
	min_vertical	= p_row - (ROW_DRAW_STAGE_LEN / 2);
	min_horizontal	= p_col - (COL_DRAW_STAGE_LEN / 2);
#if 0
	max_vertical	= p_row + (ROW_DRAW_STAGE_LEN / 2);
	max_horizontal	= p_col + (COL_DRAW_STAGE_LEN / 2);

	LOG_DEBUG("Drawing with limits: h_min %d, h_max %d, v_min %d, v_max %d\n",
			min_horizontal, max_horizontal, min_vertical, max_vertical);
#endif

	cursor_horizontal = min_horizontal;
	cursor_vertical = min_vertical;

	for (int i = 0; i < ROW_DRAW_STAGE_LEN; i++) {
		for (int k = 0; k < COL_DRAW_STAGE_LEN; k++) {
			add_tile_to_slice(i, k, cursor_vertical, cursor_horizontal);
			cursor_horizontal++;
		}

		cursor_horizontal = min_horizontal;
		cursor_vertical++;
	}
}

static void add_tile_to_slice(
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

void set_stage_room(int start_row, int start_col, int len_vertical, int len_horizontal)
{
	assert(start_row >= 0);
	assert(start_col >= 0);
	assert((start_row + len_vertical) <= STAGE_SIZE_VERTICAL);
	assert((start_col + len_horizontal) <= STAGE_SIZE_HORIZONTAL);

	for (int i = start_row; i < len_vertical; i++) {
		for (int k = start_col; k < len_horizontal; k++) {
			if (is_corner(i, k)) {
				g_stage[i][k].terrain = g_all_terrains[ALL_TERRAINS_COLUMN];
			} else if (is_horizontal_edge(i, k)) {
				g_stage[i][k].terrain = g_all_terrains[ALL_TERRAINS_WALL_HORIZONTAL];
			} else if (is_vertical_edge(i, k)) {
				g_stage[i][k].terrain = g_all_terrains[ALL_TERRAINS_WALL_VERTICAL];
			} else {
				g_stage[i][k].terrain = g_all_terrains[ALL_TERRAINS_FLOOR];
			}
		}
	}
}

static void load_stage_hideout(void)
{
	const int hideout_len_vertical = STAGE_SIZE_VERTICAL;
	const int hideout_len_horizontal = STAGE_SIZE_HORIZONTAL;

	set_stage_room(0, 0, hideout_len_vertical, hideout_len_horizontal);

	spawn_player(2, 2, get_all_actors());
	spawn_actor(
			"merchant",
			5, 8,
			ICON_MERCHANT,
			despawn_actor,
			initiate_trade,
			100);
	spawn_actor(
			"portal",
			5, 14,
			ICON_PORTAL,
			despawn_actor,
			transport_to_stage,
			100);
	spawn_item_drop(4, 4, get_all_actors(), get_all_items(), 2, SPAWN_ITEM_TYPE_CONSUMABLE);
	spawn_item_drop(5, 5, get_all_actors(), get_all_items(), 2, SPAWN_ITEM_TYPE_EQUIPMENT);

	set_stage_name("Hideout");
}

static void load_stage_dungeon(void)
{
	const int dungeon_len_vertical = STAGE_SIZE_VERTICAL;
	const int dungeon_len_horizontal = STAGE_SIZE_HORIZONTAL;

	set_stage_room(0, 0, dungeon_len_vertical, dungeon_len_horizontal);

	spawn_player(2, 2, get_all_actors());

	set_stage_name("Dungeon");
}

void unload_stage(void)
{
	despawn_all_actors();
	for (int i = 0; i < STAGE_SIZE_VERTICAL; i++) {
		for (int k = 0; k < STAGE_SIZE_HORIZONTAL; k++) {
			g_stage[i][k].terrain = g_all_terrains[ALL_TERRAINS_VOID];
			assert(0 == g_stage[i][k].occupant); // Verify actor despawn
		}
	}
	for (int i = 0; i < STAGE_SLICE_SIZE_HORIZONTAL; i++) {
		for (int k = 0; k < STAGE_SLICE_SIZE_VERTICAL; k++) {
			g_stage_slice[i][k].terrain = g_all_terrains[ALL_TERRAINS_VOID];

			// Actor despawn only sets g_stage to point to 0. Actor
			// coordinates apply to stage, not stage slice. When
			// stage slice index is outside of stage, it is not
			// copied from stage, but instead just set to point to
			// void terrain. Could also set occupant to 0 in
			// add_tile_to_slice(), but fearing this to hide bugs.
			g_stage_slice[i][k].occupant = 0;
		}
	}
}
