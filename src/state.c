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

#include <SDL/SDL_mixer.h>

#include "main.h"
#include "game.h"
#include "state.h"
#include "level.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

#define TITLE_IMG "data/png/title.png"
#define READY_IMG "data/png/ready.png"
#define SET_IMG   "data/png/set.png"
#define GO_IMG    "data/png/go.png"
#define GOAL_IMG  "data/png/goal.png"
#define FALL_IMG  "data/png/fall.png"
#define TIME_IMG  "data/png/time.png"
#define OVER_IMG  "data/png/over.png"
#define PAUSE_IMG "data/png/pause.png"

#define TITLE_SND "data/ogg/title.ogg"
#define READY_SND "data/ogg/ready.ogg"
#define SET_SND   "data/ogg/set.ogg"
#define GO_SND    "data/ogg/go.ogg"
#define GOAL_SND  "data/ogg/goal.ogg"
#define FALL_SND  "data/ogg/fall.ogg"
#define TIME_SND  "data/ogg/time.ogg"
#define OVER_SND  "data/ogg/over.ogg"
#define PAUSE_SND "data/ogg/pause.ogg"

#define STATE_CH 3

/*---------------------------------------------------------------------------*/

static double goto_time    = 0.0;
static struct state *state = NULL;

void goto_state(struct state *st)
{
    if (state && state->leave)
        state->leave();

    state     = st;
    goto_time = SDL_GetTicks() / 1000.0;

    if (state && state->enter)
        state->enter();
}

double time_state(void)
{
    return SDL_GetTicks() / 1000.0 - goto_time;
}

/*---------------------------------------------------------------------------*/

void st_paint(void)
{
    if (state && state->paint) state->paint();
}

int st_timer(double t)
{
    return (state && state->timer) ? state->timer(t) : 1;
}

int st_point(int x, int y)
{
    return (state && state->point) ? state->point(x, y) : 1;
}

int st_click(int b)
{
    return (state && state->click) ? state->click(b) : 1;
}

int st_keybd(int c)
{
    return (state && state->keybd) ? state->keybd(c) : 1;
}

/*---------------------------------------------------------------------------*/

static void title_enter(void)
{
    static Mix_Chunk *snd = NULL;

    if (snd || (snd = Mix_LoadWAV(TITLE_SND)))
        Mix_PlayChannel(STATE_CH, snd, 0);
}

static void title_leave(void)
{
    game_start();
}

static void title_paint(void)
{
    static struct image img;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (image_test(&img) == 0)
        image_load(&img, TITLE_IMG);
    if (image_test(&img) == 1)
        image_rect(&img, 0.0, 0.25, 1.0, 0.75, 1.0);
}

static int title_click(int d)
{
    if (d == 0)
    {
        level_init();
        goto_state(&st_ready);
    }
    return 1;
}

static int title_keybd(int c)
{
    if (c == SDLK_F1)
        return 0;

    return 1;
}

/*---------------------------------------------------------------------------*/

static void ready_enter(void)
{
    static Mix_Chunk *snd = NULL;

    if (snd || (snd = Mix_LoadWAV(READY_SND)))
        Mix_PlayChannel(STATE_CH, snd, 0);

    game_update_fly(1.0);
}

static void ready_paint(void)
{
    static struct image ready_i;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();

    if (image_test(&ready_i) == 0)
        image_load(&ready_i, READY_IMG);
    if (image_test(&ready_i) == 1)
        image_rect(&ready_i, 0.0, 0.33, 1.0, 0.66, 1.0 - time_state());
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
    static Mix_Chunk *snd = NULL;

    if (snd || (snd = Mix_LoadWAV(SET_SND)))
        Mix_PlayChannel(STATE_CH, snd, 0);
}

static void set_paint(void)
{
    static struct image set_i;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();

    if (image_test(&set_i) == 0)
        image_load(&set_i, SET_IMG);
    if (image_test(&set_i) == 1)
        image_rect(&set_i, 0.25, 0.33, 0.75, 0.66, 1.0 - time_state());
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
    static Mix_Chunk *snd = NULL;

    if (snd || (snd = Mix_LoadWAV(GO_SND)))
        Mix_PlayChannel(STATE_CH, snd, 0);

    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_ShowCursor(SDL_DISABLE);

    SDL_WarpMouse((Uint16) (width / 2), (Uint16) (height / 2));
}

static void play_leave(void)
{
    SDL_ShowCursor(SDL_ENABLE);
    SDL_WM_GrabInput(SDL_GRAB_OFF);
}

static void play_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();
    game_render_hud(fps());

    if (time_state() < 1.0)
    {
        static struct image go_i;

        if (image_test(&go_i) == 0)
            image_load(&go_i, GO_IMG);
        if (image_test(&go_i) == 1)
            image_rect(&go_i, 0.25, 0.33, 0.75, 0.66, 1.0 - time_state());
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
    if (c == SDLK_ESCAPE)
        goto_state(&st_pause);

    return 1;
}

/*---------------------------------------------------------------------------*/

static void goal_enter(void)
{
    static Mix_Chunk *snd = NULL;

    if (snd || (snd = Mix_LoadWAV(GOAL_SND)))
        Mix_PlayChannel(STATE_CH, snd, 0);
}

static void goal_paint(void)
{
    static struct image img;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();

    if (image_test(&img) == 0)
        image_load(&img, GOAL_IMG);
    if (image_test(&img) == 1)
        image_rect(&img, 0.0, 0.33, 1.0, 0.66, 1.0);
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
        if (level_pass())
            goto_state(&st_ready);
        else
            goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void fall_enter(void)
{
    static Mix_Chunk *snd = NULL;

    if (snd || (snd = Mix_LoadWAV(FALL_SND)))
        Mix_PlayChannel(STATE_CH, snd, 0);
}

static void fall_paint(void)
{
    static struct image img;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();

    if (image_test(&img) == 0)
        image_load(&img, FALL_IMG);
    if (image_test(&img) == 1)
        image_rect(&img, 0.0, 0.33, 1.0, 0.66, 1.0);
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
        if (level_fail())
            goto_state(&st_ready);
        else
            goto_state(&st_over);
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

static void time_enter(void)
{
    static Mix_Chunk *snd = NULL;

    if (snd || (snd = Mix_LoadWAV(TIME_SND)))
        Mix_PlayChannel(STATE_CH, snd, 0);
}

static void time_paint(void)
{
    static struct image img;

    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    game_render_env();

    if (image_test(&img) == 0)
        image_load(&img, TIME_IMG);
    if (image_test(&img) == 1)
        image_rect(&img, 0.0, 0.33, 1.0, 0.66, 1.0);
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
        if (level_fail())
            goto_state(&st_ready);
        else
            goto_state(&st_over);
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

static void over_enter(void)
{
    static Mix_Chunk *snd = NULL;

    if (snd || (snd = Mix_LoadWAV(OVER_SND)))
        Mix_PlayChannel(STATE_CH, snd, 0);
}

static void over_paint(void)
{
    static struct image img;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (image_test(&img) == 0)
        image_load(&img, OVER_IMG);
    if (image_test(&img) == 1)
        image_rect(&img, 0.0, 0.25, 1.0, 0.75, 1.0);
}

static int over_timer(double dt)
{
    if (dt > 0.0 && time_state() > 3.0)
        goto_state(&st_title);

    return 1;
}

static int over_keybd(int c)
{
    if (c == SDLK_F1)
        goto_state(&st_title);

    return 1;
}

/*---------------------------------------------------------------------------*/

static void pause_enter(void)
{
    static Mix_Chunk *snd = NULL;

    if (snd || (snd = Mix_LoadWAV(PAUSE_SND)))
        Mix_PlayChannel(STATE_CH, snd, 0);
}

static void pause_paint(void)
{
    static struct image pause_i;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();

    if (image_test(&pause_i) == 0)
        image_load(&pause_i, PAUSE_IMG);
    if (image_test(&pause_i) == 1)
        image_rect(&pause_i, 0.0, 0.33, 1.0, 0.66, 0.5);
}

static int pause_keybd(int c)
{
    if (c == SDLK_F1)
        goto_state(&st_title);

    if (c == SDLK_ESCAPE)
        goto_state(&st_play);

    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_title = {
    title_enter,
    title_leave,
    title_paint,
    NULL,
    NULL,
    title_click,
    title_keybd
};

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

struct state st_over = {
    over_enter,
    NULL,
    over_paint,
    over_timer,
    NULL,
    NULL,
    over_keybd
};

struct state st_pause = {
    pause_enter,
    NULL,
    pause_paint,
    NULL,
    NULL,
    NULL,
    pause_keybd
};

