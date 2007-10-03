#ifndef LEVELS_H
#define LEVELS_H

#include "game.h"

/*---------------------------------------------------------------------------*/

const struct level_game *curr_lg(void);

int  level_replay(const char *);
int  level_play(const struct level *, int);
void level_stat(int, int, int);
void level_stop(void);
int  level_next(void);
int  level_same(void);

int  count_extra_balls(int, int);

void level_update_player_name(void);

/*---------------------------------------------------------------------------*/

#endif
