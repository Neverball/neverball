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
#include "menu.h"
#include "state.h"
#include "level.h"
#include "image.h"
#include "audio.h"

/*---------------------------------------------------------------------------*/

#define IMG_TITLE "data/png/title.png"
#define IMG_START "data/png/grid.png"
#define IMG_CONF  "data/png/conf.png"
#define IMG_READY "data/png/ready.png"
#define IMG_SET   "data/png/set.png"
#define IMG_GO    "data/png/go.png"
#define IMG_GOAL  "data/png/goal.png"
#define IMG_FALL  "data/png/fall.png"
#define IMG_TIME  "data/png/time.png"
#define IMG_OVER  "data/png/over.png"
#define IMG_PAUSE "data/png/pause.png"

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

#define TITLE_PLAY 0
#define TITLE_CONF 1
#define TITLE_EXIT 2

static struct menu title_menu;
static struct item title_item[3] = {
    { 128, 172, 256, 64, 0, TITLE_PLAY },
    { 128, 108, 256, 64, 0, TITLE_CONF },
    { 128,  44, 256, 64, 0, TITLE_EXIT },
};

static void title_enter(void)
{
    audio_play(AUD_TITLE, 1.f);

    title_item[TITLE_PLAY].lit = 1;
    title_item[TITLE_CONF].lit = 0;
    title_item[TITLE_EXIT].lit = 0;

    menu_init(&title_menu, IMG_TITLE, title_item, 3, 0);
}

static void title_leave(void)
{
    menu_free(&title_menu);
}

static void title_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    menu_paint(&title_menu, 1.0);
}

static int title_point(int x, int y)
{
    menu_point(&title_menu, x, y);
    return 1;
}

static int title_click(int d)
{
    switch (menu_click(&title_menu, d))
    {
    case TITLE_PLAY:
        goto_state(&st_start);
        break;
    case TITLE_CONF:
        audio_play(AUD_MENU, 1.f);
        goto_state(&st_conf);
        break;
    case TITLE_EXIT:
        return 0;
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

#define CONF_FULL      0
#define CONF_WIN       1
#define CONF_1600x1200 2
#define CONF_1280x1024 3
#define CONF_1024x768  4
#define CONF_800x600   5
#define CONF_640x480   6
#define CONF_TEX_HI    7
#define CONF_TEX_LO    8
#define CONF_GEO_HI    9
#define CONF_GEO_LO   10
#define CONF_AUD_HI   11
#define CONF_AUD_LO   12
#define CONF_BACK     13

static struct menu conf_menu;
static struct item conf_item[14] = {
    {   0, 320, 256, 48, 0, CONF_FULL      },
    {   0, 272, 256, 48, 0, CONF_WIN       },
    { 256, 416, 256, 48, 0, CONF_1600x1200 },
    { 256, 368, 256, 48, 0, CONF_1280x1024 },
    { 256, 320, 256, 48, 0, CONF_1024x768  },
    { 256, 272, 256, 48, 0, CONF_800x600   },
    { 256, 224, 256, 48, 0, CONF_640x480   },
    { 256, 176, 128, 48, 0, CONF_TEX_HI    },
    { 384, 176, 128, 48, 0, CONF_TEX_LO    },
    { 256, 128, 128, 48, 0, CONF_GEO_HI    },
    { 384, 128, 128, 48, 0, CONF_GEO_LO    },
    { 256,  80, 128, 48, 0, CONF_AUD_HI    },
    { 384,  80, 128, 48, 0, CONF_AUD_LO    },
    { 256,  32, 256, 48, 0, CONF_BACK      },
};

static void conf_enter(void)
{
    conf_item[CONF_FULL].lit      =  (main_mode & SDL_FULLSCREEN);
    conf_item[CONF_WIN].lit       = !(main_mode & SDL_FULLSCREEN);
    conf_item[CONF_1600x1200].lit =  (main_width  == 1600);
    conf_item[CONF_1280x1024].lit =  (main_width  == 1280);
    conf_item[CONF_1024x768].lit  =  (main_width  == 1024);
    conf_item[CONF_800x600].lit   =  (main_width  == 800);
    conf_item[CONF_640x480].lit   =  (main_width  == 640);
    conf_item[CONF_TEX_HI].lit    =  (image_scale == 1);
    conf_item[CONF_TEX_LO].lit    =  (image_scale == 2);
    conf_item[CONF_GEO_HI].lit    =  (main_geom   == 1);
    conf_item[CONF_GEO_LO].lit    =  (main_geom   == 0);
    conf_item[CONF_AUD_HI].lit    =  (main_rate   == 44100);
    conf_item[CONF_AUD_LO].lit    =  (main_rate   == 22050);
    conf_item[CONF_BACK].lit      =  0;

    menu_init(&conf_menu, IMG_CONF, conf_item, 14, 0);
}

static void conf_leave(void)
{
    menu_free(&conf_menu);
}

static void conf_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    menu_paint(&conf_menu, 1.0);
}

static int conf_point(int x, int y)
{
    menu_point(&conf_menu, x, y);
    return 1;
}

static int conf_click(int d)
{
    int s = 1;
    int c = menu_click(&conf_menu, d);

    switch (c)
    {
    case CONF_FULL:
        goto_state(&st_null);
        s = set_mode(main_width, main_height, SDL_OPENGL | SDL_FULLSCREEN);
        goto_state(&st_conf);
        break;
    case CONF_WIN:
        goto_state(&st_null);
        s = set_mode(main_width, main_height, SDL_OPENGL);
        goto_state(&st_conf);
        break;
    case CONF_1600x1200:
        goto_state(&st_null);
        s = set_mode(1600, 1200, main_mode);
        goto_state(&st_conf);
        break;
    case CONF_1280x1024:
        goto_state(&st_null);
        s = set_mode(1280, 1024, main_mode);
        goto_state(&st_conf);
        break;
    case CONF_1024x768:
        goto_state(&st_null);
        s = set_mode(1024, 768, main_mode);
        goto_state(&st_conf);
        break;
    case CONF_800x600:
        goto_state(&st_null);
        s = set_mode(800, 600, main_mode);
        goto_state(&st_conf);
        break;
    case CONF_640x480:
        goto_state(&st_null);
        s = set_mode(640, 480, main_mode);
        goto_state(&st_conf);
        break;
    case CONF_TEX_HI:
        goto_state(&st_null);
        image_scale = 1;
        goto_state(&st_conf);
        break;
    case CONF_TEX_LO:
        goto_state(&st_null);
        image_scale = 2;
        goto_state(&st_conf);
        break;
    case CONF_GEO_HI:
        goto_state(&st_null);
        main_geom = 1;
        goto_state(&st_conf);
        break;
    case CONF_GEO_LO:
        goto_state(&st_null);
        main_geom = 0;
        goto_state(&st_conf);
        break;
    case CONF_AUD_HI:
        audio_free();
        audio_init((main_rate = 44100), (main_buff = 2048));
        goto_state(&st_conf);
        break;
    case CONF_AUD_LO:
        audio_free();
        audio_init((main_rate = 22050), (main_buff = 1024));
        goto_state(&st_conf);
        break;
    case CONF_BACK:
        goto_state(&st_title);
        break;
    }
    if (c >= 0) audio_play(AUD_MENU, 1.f);

    return s;
}

static int conf_keybd(int c)
{
    if (c == SDLK_F1)
        goto_state(&st_title);

    return 1;
}

/*---------------------------------------------------------------------------*/

#define START_BACK 99

static struct menu start_menu;
static struct item start_item[65];

static void start_enter(void)
{
    int i, r, c;

    level_init();

    for (r = 0, i = 0; r < 8; r++)
        for (c = 0; c < 8; c++, i++)
        {
            start_item[i].x =      c  * 64;
            start_item[i].y = (7 - r) * 48 + 40;
            start_item[i].w = 64;
            start_item[i].h = 48;
            start_item[i].lit = (i < count) ? 0 : -1;
            start_item[i].val = i;
        }

    start_item[64].x   = 400;
    start_item[64].y   = 424;
    start_item[64].w   = 112;
    start_item[64].h   = 48;
    start_item[64].val = START_BACK;

    menu_init(&start_menu, IMG_START, start_item, 65, 0);
}

static void start_leave(void)
{
    menu_free(&start_menu);
}

static void start_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    menu_paint(&start_menu, 1.0);
}

static int start_point(int x, int y)
{
    menu_point(&start_menu, x, y);
    return 1;
}

static int start_click(int d)
{
    int c = menu_click(&start_menu, d);

    if (c >= 0)
    {
        if (c == START_BACK)
            goto_state(&st_title);
        else
        {
            level_goto(c);
            goto_state(&st_level);
        }
    }
    return 1;
}

static int start_keybd(int c)
{
    if (c == SDLK_F1)
        goto_state(&st_title);

    return 1;
}

/*---------------------------------------------------------------------------*/

static void level_enter(void)
{
    audio_play(AUD_LEVEL, 1.f);

    game_update_fly(1.0);
}

static void level_leave(void)
{
}

static void level_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();
    game_render_num(level + 1);
}

static int level_timer(double dt)
{
    double t = time_state();

    game_update_fly(1.0 - 0.33 * t);

    if (dt > 0.0 && t > 1.0)
        goto_state(&st_ready);

    return 1;
}

/*---------------------------------------------------------------------------*/

static struct menu ready_menu;

static void ready_enter(void)
{
    audio_play(AUD_READY, 1.f);
    menu_init(&ready_menu, IMG_READY, NULL, 0, 0);
}

static void ready_leave(void)
{
    menu_free(&ready_menu);
}

static void ready_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();
    game_render_hud();
    menu_paint(&ready_menu, 1.0 - time_state());
}

static int ready_timer(double dt)
{
    double t = time_state();

    game_update_fly(0.66 - 0.33 * t);

    if (dt > 0.0 && t > 1.0)
        goto_state(&st_set);

    return 1;
}

/*---------------------------------------------------------------------------*/

static struct menu set_menu;

static void set_enter(void)
{
    audio_play(AUD_SET, 1.f);
    menu_init(&set_menu, IMG_SET, NULL, 0, 0);
}

static void set_leave(void)
{
    menu_free(&set_menu);
}

static void set_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();
    game_render_hud();
    menu_paint(&set_menu, 1.0 - time_state());
}

static int set_timer(double dt)
{
    double t = time_state();

    game_update_fly(0.33 - 0.33 * t);

    if (dt > 0.0 && t > 1.0)
        goto_state(&st_play);

    return 1;
}

/*---------------------------------------------------------------------------*/

static struct menu play_menu;

static void play_enter(void)
{
    audio_play(AUD_GO, 1.f);
    menu_init(&play_menu, IMG_GO, NULL, 0, 0);

    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_WarpMouse((Uint16) (main_width / 2), (Uint16) (main_height / 2));
}

static void play_leave(void)
{
    SDL_ShowCursor(SDL_ENABLE);
    SDL_WM_GrabInput(SDL_GRAB_OFF);

    menu_free(&play_menu);
}

static void play_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();
    game_render_hud();

    if (time_state() < 1.0)
        menu_paint(&play_menu, 1.0 - time_state());
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
    int dx = x - main_width  / 2;
    int dy = y - main_height / 2;

    if (dx || dy)
    {
        SDL_WarpMouse((Uint16) (main_width / 2), (Uint16) (main_height / 2));
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

static struct menu goal_menu;

static void goal_enter(void)
{
    audio_play(AUD_GOAL, 1.f);
    menu_init(&goal_menu, IMG_GOAL, NULL, 0, 0);
}

static void goal_leave(void)
{
    menu_free(&goal_menu);
}

static void goal_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();
    menu_paint(&goal_menu, 1.0);
}

static int goal_timer(double dt)
{
    double g[3] = { 0.0, 9.8, 0.0 };

    if (time_state() < 2.0)
        game_update_env(g, dt);
    else
    {
        if (level_pass())
            goto_state(&st_level);
        else
            goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static struct menu fall_menu;

static void fall_enter(void)
{
    audio_play(AUD_FALL, 1.f);
    menu_init(&fall_menu, IMG_FALL, NULL, 0, 0);
}

static void fall_leave(void)
{
    menu_free(&fall_menu);
}

static void fall_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();
    game_render_hud();
    menu_paint(&fall_menu, 1.0);
}

static int fall_timer(double dt)
{
    double g[3] = { 0.0, -9.8, 0.0 };

    if (time_state() < 2.0)
        game_update_env(g, dt);
    else
    {
        if (level_fail())
            goto_state(&st_level);
        else
            goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static struct menu time_menu;

static void time_enter(void)
{
    audio_play(AUD_TIME, 1.f);
    menu_init(&time_menu, IMG_TIME, NULL, 0, 0);
}

static void time_leave(void)
{
    menu_free(&time_menu);
}

static void time_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();
    game_render_hud();
    menu_paint(&time_menu, 1.0);
}

static int time_timer(double dt)
{
    double g[3] = { 0.0, -9.8, 0.0 };

    if (time_state() < 2.0)
        game_update_env(g, dt);
    else
    {
        if (level_fail())
            goto_state(&st_level);
        else
            goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static struct menu over_menu;

static void over_enter(void)
{
    audio_play(AUD_OVER, 1.f);
    menu_init(&over_menu, IMG_OVER, NULL, 0, 0);
}

static void over_leave(void)
{
    menu_free(&over_menu);
    level_free();
}

static void over_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();
    menu_paint(&over_menu, 1.0);
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

static struct menu pause_menu;

static void pause_enter(void)
{
    audio_play(AUD_PAUSE, 1.f);
    menu_init(&pause_menu, IMG_PAUSE, NULL, 0, 0);
}

static void pause_leave(void)
{
    menu_free(&pause_menu);
}

static void pause_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();
    menu_paint(&pause_menu, 0.5);
}

static int pause_keybd(int c)
{
    if (c == SDLK_F1)
        goto_state(&st_over);

    if (c == SDLK_ESCAPE)
        goto_state(&st_play);

    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_null = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

struct state st_title = {
    title_enter,
    title_leave,
    title_paint,
    NULL,
    title_point,
    title_click,
    title_keybd
};

struct state st_start = {
    start_enter,
    start_leave,
    start_paint,
    NULL,
    start_point,
    start_click,
    start_keybd
};

struct state st_conf = {
    conf_enter,
    conf_leave,
    conf_paint,
    NULL,
    conf_point,
    conf_click,
    conf_keybd
};

struct state st_level = {
    level_enter,
    level_leave,
    level_paint,
    level_timer,
    NULL,
    NULL,
    NULL
};

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

struct state st_over = {
    over_enter,
    over_leave,
    over_paint,
    over_timer,
    NULL,
    NULL,
    over_keybd
};

struct state st_pause = {
    pause_enter,
    pause_leave,
    pause_paint,
    NULL,
    NULL,
    NULL,
    pause_keybd
};

