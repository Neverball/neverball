#include <stdlib.h>
#include <GL/gl.h>

#include <glw.h>
#include <aux.h>

#include "main.h"
#include "time.h"
#include "goal.h"
#include "fall.h"
#include "game.h"
#include "play.h"
#include "pause.h"

#define READY_S "data/png/ready.png"
#define SET_S   "data/png/set.png"
#define GO_S    "data/png/go.png"

static void  *ready_p = NULL;
static GLuint ready_o = 0;
static void  *set_p   = NULL;
static GLuint set_o   = 0;
static void  *go_p    = NULL;
static GLuint go_o    = 0;

/*---------------------------------------------------------------------------*/

static void ready_enter(void)
{
    if (ready_p == NULL)
    {
        int w, h, b;

        ready_p = aux_load_png(READY_S, &w, &h, &b);
        ready_o = aux_make_tex(ready_p,  w,  h,  b);
    }

    game_update_fly(1.0);
}

static void ready_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

    game_render_env();

    aux_proj_identity();
    aux_draw_tex(ready_o, 0.0, 0.33, 1.0, 0.66, 1.0 - time_state());
}

static int ready_timer(double dt)
{
    double t = time_state();

    game_update_fly(1.0 - 0.5 * t);

    if (t > 1.0)
        goto_state(&st_set);

    return 1;
}

/*---------------------------------------------------------------------------*/

static void set_enter(void)
{
    if (set_p == NULL)
    {
        int w, h, b;

        set_p = aux_load_png(SET_S, &w, &h, &b);
        set_o = aux_make_tex(set_p,  w,  h,  b);
    }
}

static void set_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

    game_render_env();

    aux_proj_identity();
    aux_draw_tex(set_o, 0.25, 0.33, 0.75, 0.66, 1.0 - time_state());
}

static int set_timer(double dt)
{
    double t = time_state();

    game_update_fly(0.5 - 0.5 * t);

    if (t > 1.0)
        goto_state(&st_play);

    return 1;
}

/*---------------------------------------------------------------------------*/

static void play_enter(void)
{
    if (go_p == NULL)
    {
        int w, h, b;

        go_p = aux_load_png(GO_S, &w, &h, &b);
        go_o = aux_make_tex(go_p,  w,  h,  b);
    }

    glw_acquire();
}

static void play_leave(void)
{
    glw_release();
}

static void play_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

    game_render_env();
    game_render_hud();

    if (time_state() < 1.0)
    {
        aux_proj_identity();
        aux_draw_tex(go_o, 0.25, 0.33, 0.75, 0.66, 1.0 - time_state());
    }
}

static int play_timer(double dt)
{
    double g[3] = { 0.0, -9.8, 0.0 };

    switch (game_update_env(g, dt))
    {
    case GAME_TIME: goto_state(&st_time); break;
    case GAME_GOAL: goto_state(&st_goal); break;
    case GAME_FALL: goto_state(&st_fall); break;
    }

    return 1;
}

static int play_point(int x, int y)
{
    game_update_pos(x, y);
    return 1;
}

static int play_keybd(int c)
{
    if (c == 27)
        goto_state(&st_pause);

    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_ready = {
    ready_enter,
    NULL,
    ready_paint,
    ready_timer,
    NULL,
    NULL,
    NULL
};

struct state st_set = {
    set_enter,
    NULL,
    set_paint,
    set_timer,
    NULL,
    NULL,
    NULL
};

struct state st_play = {
    play_enter,
    play_leave,
    play_paint,
    play_timer,
    play_point,
    NULL,
    play_keybd
};

