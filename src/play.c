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
#include "play.h"
#include "stop.h"
#include "pause.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

#define READY_S "data/png/ready.png"
#define SET_S   "data/png/set.png"
#define GO_S    "data/png/go.png"

static struct image ready_i, *ready_p = &ready_i;
static struct image set_i,   *set_p   = &set_i;
static struct image go_i,    *go_p    = &go_i;

/*---------------------------------------------------------------------------*/

static void ready_enter(void)
{
    game_update_fly(1.0);
    image_load(ready_p, READY_S);
}

static void ready_leave(void)
{
    image_free(ready_p);
}

static void ready_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    game_render_env();
    image_rect(ready_p, 0.0, 0.33, 1.0, 0.66, 1.0 - time_state());
}

static int ready_timer(double dt)
{
    double t = time_state();

    game_update_fly(1.0 - 0.5 * t);

    if (dt > 0.0 && t > 1.0)
        goto_state(&st_set);

    return 1;
}

/*---------------------------------------------------------------------------*/

static void set_enter(void)
{
    image_load(set_p, SET_S);
}

static void set_leave(void)
{
    image_free(set_p);
}

static void set_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    game_render_env();
    image_rect(set_p, 0.25, 0.33, 0.75, 0.66, 1.0 - time_state());
}

static int set_timer(double dt)
{
    double t = time_state();

    game_update_fly(0.5 - 0.5 * t);

    if (dt > 0.0 && t > 1.0)
        goto_state(&st_play);

    return 1;
}

/*---------------------------------------------------------------------------*/

static void play_enter(void)
{
    image_load(go_p, GO_S);

    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_ShowCursor(SDL_DISABLE);

    SDL_WarpMouse((Uint16) (width / 2), (Uint16) (height / 2));
}

static void play_leave(void)
{
    SDL_ShowCursor(SDL_ENABLE);
    SDL_WM_GrabInput(SDL_GRAB_OFF);

    image_free(go_p);
}

static void play_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    game_render_env();
    game_render_hud();

    if (time_state() < 1.0)
        image_rect(go_p, 0.25, 0.33, 0.75, 0.66, 1.0 - time_state());
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
    int dx = x - width  / 2;
    int dy = y - height / 2;

    if (dx || dy)
    {
        SDL_WarpMouse((Uint16) (width / 2), (Uint16) (height / 2));
        game_update_pos(dx, dy);
    }
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
    ready_leave,
    ready_paint,
    ready_timer,
    NULL,
    NULL,
    NULL
};

struct state st_set = {
    set_enter,
    set_leave,
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

