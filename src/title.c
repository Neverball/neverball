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
#include "play.h"
#include "game.h"
#include "title.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

#define TITLE_S    "data/png/title.png"
#define GAMEOVER_S "data/png/gameover.png"

static struct image title_i,    *title_p    = &title_i;
static struct image gameover_i, *gameover_p = &gameover_i;

/*---------------------------------------------------------------------------*/

#define MAXSTR 256
#define MAXLVL 8

struct level
{
    char file[MAXSTR];
    int  time;
};

static struct level level_v[MAXLVL];
static int          level_i = 0;
static int          level_n = 0;

/*---------------------------------------------------------------------------*/

static void gameover_enter(void)
{
    image_load(gameover_p, GAMEOVER_S);
}

static void gameover_leave(void)
{
    image_free(gameover_p);
}

static void gameover_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    image_rect(gameover_p, 0.0, 0.25, 1.0, 0.75, 1.0);
}

static int gameover_timer(double dt)
{
    if (dt > 0.0 && time_state() > 3.0)
        goto_state(&st_title);

    return 1;
}

static int gameover_keybd(int c)
{
    if (c == SDLK_F1)
        goto_state(&st_title);

    return 1;
}

/*---------------------------------------------------------------------------*/

void level_init(void)
{
    if (level_n == 0)
    {
        FILE *fin = fopen("data/levels.txt", "r");

        if (fin)
        {
            while (fscanf(fin, "%s %d",
                           level_v[level_n].file,
                          &level_v[level_n].time) == 2)
                level_n++;

            fclose(fin);
        }
    }

    level_i = 0;
}

void level_pass(void)
{
    level_i++;
}

static void level_enter(void)
{
    game_free();

    if (level_i < level_n)
    {
        game_load(level_v[level_i].file,
                  level_v[level_i].time);

        goto_state(&st_ready);
    }
    else
        goto_state(&st_gameover);
}

/*---------------------------------------------------------------------------*/

static void title_enter(void)
{
    image_load(title_p, TITLE_S);
}

static void title_leave(void)
{
    image_free(title_p);

    level_init();
    game_start();
}

static void title_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    image_rect(title_p, 0.0, 0.25, 1.0, 0.75, 1.0);
}

static int title_click(int d)
{
    if (d == 0)
        goto_state(&st_level);

    return 1;
}

static int title_keybd(int c)
{
    if (c == SDLK_F1)
        return 0;

    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_gameover = {
    gameover_enter,
    gameover_leave,
    gameover_paint,
    gameover_timer,
    NULL,
    NULL,
    gameover_keybd
};

struct state st_level = {
    level_enter,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

struct state st_title = {
    title_enter,
    title_leave,
    title_paint,
    NULL,
    NULL,
    title_click,
    title_keybd
};
