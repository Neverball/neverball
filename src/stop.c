/*   
 *   Copyright (C) 2003 Robert Kooima
 *
 *   SUPER EMPTY BALL is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by the
 *   Free Software Foundation;  either version 2 of the  License, or (at your
 *   option) any later version.
 *
 *   This program  is distributed  in the  hope that it  will be  useful, but
 *   WITHOUT   ANY   WARRANTY;  without   even   the   implied  warranty   of
 *   MERCHANTABILITY  or  FITNESS FOR  A  PARTICULAR  PURPOSE.   See the  GNU
 *   General Public License for more details.
 */

#include "main.h"
#include "game.h"
#include "title.h"
#include "pause.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

#define GOAL_S "data/png/goal.png"
#define FALL_S "data/png/fall.png"
#define TIME_S "data/png/time.png"

static struct image goal_i, *goal_p = &goal_i;
static struct image fall_i, *fall_p = &fall_i;
static struct image time_i, *time_p = &time_i;

/*---------------------------------------------------------------------------*/

static void goal_enter(void)
{
    image_load(goal_p, GOAL_S);
}

static void goal_leave(void)
{
    image_free(goal_p);
}

static void goal_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    game_render_env();
    image_rect(goal_p, 0.0, 0.33, 1.0, 0.66, 1.0);
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
    image_load(fall_p, FALL_S);
}

static void fall_leave(void)
{
    image_free(fall_p);
}

static void fall_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    game_render_env();
    image_rect(fall_p, 0.0, 0.33, 1.0, 0.66, 1.0);
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
    image_load(time_p, TIME_S);
}

static void time_leave(void)
{
    image_free(time_p);
}

static void time_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    game_render_env();
    image_rect(time_p, 0.0, 0.33, 1.0, 0.66, 1.0);
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
    goal_leave,
    goal_paint,
    goal_timer,
    NULL,
    NULL,
    NULL,
};

struct state st_fall = {
    fall_enter,
    fall_leave,
    fall_paint,
    fall_timer,
    NULL,
    NULL,
    NULL
};

struct state st_time = {
    time_enter,
    time_leave,
    time_paint,
    time_timer,
    NULL,
    NULL,
    NULL,
};
