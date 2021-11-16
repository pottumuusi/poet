#ifndef STAGE_H_DEFINED
#define STAGE_H_DEFINED

#include "actor.h"

#define ROW_STAGE_ZERO 3
#define COL_STAGE_ZERO 5

#define ROW_STAGE_NAME_ZERO 1
#define COL_STAGE_NAME_ZERO COL_STAGE_ZERO

#define ROW_MAX 15 // TODO rename to ROW_STAGE_MAX
#define COL_MAX 25

#define STAGE_NAME_SIZE 128

struct terrain {
	char icon;
	const char name[32];
	char traversable;
};

struct stage_shard {
	struct terrain* terrain;
	struct actor* occupant;
};

extern struct stage_shard g_stage[ROW_MAX][COL_MAX];
extern char g_stage_name[STAGE_NAME_SIZE];

#endif
