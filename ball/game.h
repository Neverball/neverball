#ifndef GAME_H
#define GAME_H

#include <stdio.h>

/*---------------------------------------------------------------------------*/

#define MAX_DT   0.01666666            /* Maximum physics update cycle       */
#define MAX_DN  16                     /* Maximum subdivisions of dt         */
#define RESPONSE 0.05f                 /* Input smoothing time               */

#define GAME_NONE 0
#define GAME_TIME 1
#define GAME_GOAL 2
#define GAME_FALL 3

/*---------------------------------------------------------------------------*/

int   game_init(const char *, const char *, const char *, int, int);
void  game_free(void);

int   curr_clock(void);
char *curr_intro(void);

void  game_draw(int, float);
int   game_step(const float[3], float, int);

void  game_set_pos(int, int);
void  game_set_x  (int);
void  game_set_z  (int);
void  game_set_rot(int);
void  game_set_fly(float);

void  game_look(float, float);

void  game_kill_fade(void);
void  game_step_fade(float);
void  game_fade(float);

int   put_game_state(FILE *);
int   get_game_state(FILE *);

/*---------------------------------------------------------------------------*/

#endif
