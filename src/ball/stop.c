/*   Copyright (C) 2003  Robert Kooima                                       */
/*                                                                           */
/*   SUPER EMPTY BALL  is  free software; you  can redistribute  it and/or   */
/*   modify  it under  the  terms  of  the  GNU General Public License  as   */
/*   published by  the Free Software Foundation;  either version 2  of the   */
/*   License, or (at your option) any later version.                         */
/*                                                                           */
/*   This program is  distributed in the hope that it  will be useful, but   */
/*   WITHOUT  ANY   WARRANTY;  without   even  the  implied   warranty  of   */
/*   MERCHANTABILITY  or FITNESS FOR  A PARTICULAR  PURPOSE.  See  the GNU   */
/*   General Public License for more details.                                */

#include <stdlib.h>
#include <GL/gl.h>

#include <glw.h>
#include <etc.h>

#include "main.h"
#include "game.h"
#include "title.h"
#include "pause.h"

/*---------------------------------------------------------------------------*/

#define GOAL_S "data/png/goal.png"
#define FALL_S "data/png/fall.png"
#define TIME_S "data/png/time.png"

static void  *goal_p = NULL;
static void  *fall_p = NULL;
static void  *time_p = NULL;

static GLuint goal_o = 0;
static GLuint fall_o = 0;
static GLuint time_o = 0;

/*---------------------------------------------------------------------------*/

static void goal_enter(void)
{
    if (goal_p == NULL)
    {
        int w, h, b;

        goal_p = etc_load_png(GOAL_S, &w, &h, &b);
        goal_o = etc_make_tex(goal_p,  w,  h,  b);
    }
}

static void goal_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

    game_render_env();

    etc_proj_identity();
    etc_draw_tex(goal_o, 0.0, 0.33, 1.0, 0.66, 1.0);
}

static int goal_timer(double dt)
{
    if (time_state() < 2.0)
    {
        double g[3] = { 0.0, 9.8, 0.0 };

        (void) game_update_env(g, dt);
    }
    else
    {
        level_pass();
        goto_state(&st_level);
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

static void fall_enter(void)
{
    if (fall_p == NULL)
    {
        int w, h, b;

        fall_p = etc_load_png(FALL_S, &w, &h, &b);
        fall_o = etc_make_tex(fall_p,  w,  h,  b);
    }
}

static void fall_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

    game_render_env();

    etc_proj_identity();
    etc_draw_tex(fall_o, 0.0, 0.33, 1.0, 0.66, 1.0);
}

static int fall_timer(double dt)
{
    if (time_state() < 2.0)
    {
        double g[3] = { 0.0, -9.8, 0.0 };

        (void) game_update_env(g, dt);
    }
    else
    {
        if (game_fail())
            goto_state(&st_level);
        else
            goto_state(&st_gameover);
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

static void time_enter(void)
{
    if (time_p == NULL)
    {
        int w, h, b;

        time_p = etc_load_png(TIME_S, &w, &h, &b);
        time_o = etc_make_tex(time_p,  w,  h,  b);
    }
}

static void time_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

    game_render_env();

    etc_proj_identity();
    etc_draw_tex(time_o, 0.0, 0.33, 1.0, 0.66, 1.0);
}

static int time_timer(double dt)
{
    if (time_state() < 2.0)
    {
        double g[3] = { 0.0, -9.8, 0.0 };

        (void) game_update_env(g, dt);
    }
    else
    {
        if (game_fail())
            goto_state(&st_level);
        else
            goto_state(&st_gameover);
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_goal = {
    goal_enter,
    NULL,
    goal_paint,
    goal_timer,
    NULL,
    NULL,
    NULL,
};

struct state st_fall = {
    fall_enter,
    NULL,
    fall_paint,
    fall_timer,
    NULL,
    NULL,
    NULL
};

struct state st_time = {
    time_enter,
    NULL,
    time_paint,
    time_timer,
    NULL,
    NULL,
    NULL,
};
