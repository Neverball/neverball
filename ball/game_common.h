#ifndef GAME_COMMON_H
#define GAME_COMMON_H

#include "lang.h"

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

#define GAME_NONE 0
#define GAME_TIME 1
#define GAME_GOAL 2
#define GAME_FALL 3

const char *status_to_str(int);

/*---------------------------------------------------------------------------*/

void game_comp_grav(float h[3], const float g[3],
                    float view_a,
                    float game_rx,
                    float game_rz);

/*---------------------------------------------------------------------------*/

#endif
