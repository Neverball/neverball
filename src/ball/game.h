#ifndef GAME_H
#define GAME_H

#include <sol.h>

/*---------------------------------------------------------------------------*/

#define GAME_NONE 0
#define GAME_TIME 1
#define GAME_GOAL 2
#define GAME_FALL 3

void game_init(void);

void game_render_hud(void);
void game_render_env(void);
int  game_update_env(const double[3], double);
void game_update_pos(int, int);
void game_update_fly(double);

void game_start(void);
void game_load(const char *, int);
int  game_fail(void);
void game_free(void);

/*---------------------------------------------------------------------------*/

#endif
