#ifndef GAME_COMMON_H
#define GAME_COMMON_H

#include "lang.h"
#include "solid_vary.h"

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
#define AUD_CLOCK   "snd/clock.ogg"

/*---------------------------------------------------------------------------*/

enum
{
    GAME_NONE = 0,

    GAME_TIME,
    GAME_GOAL,
    GAME_FALL,

    GAME_MAX
};

const char *status_to_str(int);

/*---------------------------------------------------------------------------*/

enum
{
    CAM_NONE = -1,

    CAM_1,
    CAM_2,
    CAM_3,

    CAM_MAX
};

const char *cam_to_str(int);

int cam_speed(int);

/*---------------------------------------------------------------------------*/

extern const float GRAVITY_UP[];
extern const float GRAVITY_DN[];

struct game_tilt
{
    float x[3], rx;
    float z[3], rz;
};

void game_tilt_init(struct game_tilt *);
void game_tilt_axes(struct game_tilt *, float view_e[3][3]);
void game_tilt_grav(float h[3], const float g[3], const struct game_tilt *);

/*---------------------------------------------------------------------------*/

struct game_view
{
    float dc;                           /* Ideal view distance above ball    */
    float dp;                           /* Ideal view distance above ball    */
    float dz;                           /* Ideal view distance behind ball   */

    float c[3];                         /* Current view center               */
    float p[3];                         /* Current view position             */
    float e[3][3];                      /* Current view reference frame      */

    float a;                            /* Ideal view rotation about Y axis  */
};

void game_view_init(struct game_view *);
void game_view_fly(struct game_view *, const struct s_vary *, float);

/*---------------------------------------------------------------------------*/

#define UPS 90
#define DT  (1.0f / (float) UPS)

/*
 * Simple fixed time step scheme.
 */

struct lockstep
{
    void (*step)(float);

    float dt;                           /* Time step length                  */
    float at;                           /* Accumulator                       */
    float ts;                           /* Time scale factor                 */
};

void lockstep_clr(struct lockstep *);
void lockstep_run(struct lockstep *, float);
void lockstep_scl(struct lockstep *, float);

#define lockstep_blend(ls) ((ls)->at / (ls)->dt)

/*---------------------------------------------------------------------------*/

extern struct s_base game_base;

int  game_base_load(const char *);
void game_base_free(const char *);

/*---------------------------------------------------------------------------*/

enum
{
    SPEED_NONE = 0,

    SPEED_SLOWEST,
    SPEED_SLOWER,
    SPEED_SLOW,
    SPEED_NORMAL,
    SPEED_FAST,
    SPEED_FASTER,
    SPEED_FASTEST,

    SPEED_MAX
};

extern float SPEED_FACTORS[];

#define SPEED_UP(s) MIN((s) + 1, SPEED_MAX - 1)
#define SPEED_DN(s) MAX((s) - 1, SPEED_NONE)

/*---------------------------------------------------------------------------*/

#endif
