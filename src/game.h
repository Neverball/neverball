#ifndef GAME_H
#define GAME_H

/*---------------------------------------------------------------------------*/

#define GAME_NONE 0
#define GAME_TIME 1
#define GAME_GOAL 2
#define GAME_FALL 3

void game_render_hud(void);
void game_render_num(int);
void game_render_env(void);

int  game_update_env(const double[3], double);
void game_update_pos(int, int);
void game_update_rot(int);
void game_update_fly(double);

void game_init(const char *, int);
void game_free(void);

/*---------------------------------------------------------------------------*/

#endif
