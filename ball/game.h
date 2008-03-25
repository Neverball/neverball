#ifndef GAME_H
#define GAME_H

#include <stdio.h>

#include "level.h"
#include "mode.h"

/*---------------------------------------------------------------------------*/

#define AUD_MENU    "snd/menu.ogg"
#define AUD_START _("snd/select.ogg")
#define AUD_READY _("snd/ready.ogg")
#define AUD_SET   _("snd/set.ogg")
#define AUD_GO    _("snd/go.ogg")
#define AUD_BALL    "snd/ball.ogg"
#define AUD_BUMPS   "snd/bumplil.ogg"
#define AUD_BUMPM   "snd/bump.ogg"
#define AUD_BUMPL   "snd/bumpbig.ogg"
#define AUD_COIN    "snd/coin.ogg"
#define AUD_TICK    "snd/tick.ogg"
#define AUD_TOCK    "snd/tock.ogg"
#define AUD_SWITCH  "snd/switch.ogg"
#define AUD_JUMP    "snd/jump.ogg"
#define AUD_GOAL    "snd/goal.ogg"
#define AUD_SCORE _("snd/record.ogg")
#define AUD_FALL  _("snd/fall.ogg")
#define AUD_TIME  _("snd/time.ogg")
#define AUD_OVER  _("snd/over.ogg")
#define AUD_GROW    "snd/grow.ogg"
#define AUD_SHRINK  "snd/shrink.ogg"

/*---------------------------------------------------------------------------*/

#define RESPONSE    0.05f              /* Input smoothing time               */
#define ANGLE_BOUND 20.0f              /* Angle limit of floor tilting       */
#define VIEWR_BOUND 10.0f              /* Maximum rate of view rotation      */
#define NO_AA       0                  /* Disable Angle Acceleration         */

/*---------------------------------------------------------------------------*/

int   game_init(const struct level *, int, int);
void  game_free(void);

int   curr_clock(void);
int   curr_coins(void);
int   curr_goal(void);

void  game_draw(int, float);
int   game_step(const float[3], float, int);

void  game_set_pos(int, int);
void  game_set_x  (int);
void  game_set_z  (int);
void  game_set_cam(int);
void  game_set_rot(float);
void  game_set_fly(float);

void  game_look(float, float);

void  game_kill_fade(void);
void  game_step_fade(float);
void  game_fade(float);

/*---------------------------------------------------------------------------*/

int input_put(FILE *);
int input_get(FILE *);

/*---------------------------------------------------------------------------*/

#endif
