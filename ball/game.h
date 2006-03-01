#ifndef GAME_H
#define GAME_H

#include <stdio.h>

/*---------------------------------------------------------------------------*/

#define AUD_MENU   1
#define AUD_START  2
#define AUD_READY  3
#define AUD_SET    4
#define AUD_GO     5
#define AUD_BALL   6
#define AUD_BUMP   7
#define AUD_COIN   8
#define AUD_TICK   9
#define AUD_TOCK   10
#define AUD_SWITCH 11
#define AUD_JUMP   12
#define AUD_GOAL   13
#define AUD_SCORE  14
#define AUD_FALL   15
#define AUD_TIME   16
#define AUD_OVER   17
#define AUD_COUNT  18

/*---------------------------------------------------------------------------*/

#define MAX_DT      0.01666666         /* Maximum physics update cycle       */
#define MAX_DN      16                 /* Maximum subdivisions of dt         */
#define RESPONSE    0.05f              /* Input smoothing time               */

#define ANGLE_BOUND 22.f               /* Angle limit of floor tilting       */
#define NO_AA       1                  /* Disable Angle Acceleration         */


#define GAME_NONE 0
#define GAME_TIME 1
#define GAME_GOAL 2
#define GAME_FALL 3

/*---------------------------------------------------------------------------*/

int   game_init(const char *, const char *, const char *, int, int);
void  game_free(void);

int   curr_clock(void);
int   curr_coins(void);
int   curr_goal(void);
char *curr_intro(void);

void  game_draw(int, float);
int   game_step(const float[3], float, int);

void  game_set_pos(int, int);
void  game_set_x  (int);
void  game_set_z  (int);
void  game_set_rot(float);
void  game_set_fly(float);

void  game_look(float, float);

void  game_kill_fade(void);
void  game_step_fade(float);
void  game_fade(float);

int   put_game_state(FILE *);
int   get_game_state(FILE *);

/*---------------------------------------------------------------------------*/

#endif
