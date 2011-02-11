#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include "fs.h"

/*---------------------------------------------------------------------------*/

enum
{
    POSE_NONE = 0,
    POSE_LEVEL,
    POSE_BALL
};

int   game_client_init(const char *);
void  game_client_free(void);
void  game_client_sync(fs_file);
void  game_client_draw(int, float);
void  game_client_blend(float);

int   curr_clock(void);
int   curr_coins(void);
int   curr_status(void);

void  game_look(float, float);

void  game_kill_fade(void);
void  game_step_fade(float);
void  game_fade(float);

void game_client_fly(float);

/*---------------------------------------------------------------------------*/

extern int game_compat_map;

/*---------------------------------------------------------------------------*/

#endif
