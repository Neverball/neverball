#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>

#include <aux.h>

#include "main.h"
#include "play.h"
#include "title.h"
#include "game.h"

/*---------------------------------------------------------------------------*/

#define GAMEOVER_S "data/png/gameover.png"

static void  *gameover_p = NULL;
static GLuint gameover_o = 0;

static void gameover_enter(void)
{
    if (gameover_p == NULL)
    {
        int w, h, b;

        gameover_p = aux_load_png(GAMEOVER_S, &w, &h, &b);
        gameover_o = aux_make_tex(gameover_p,  w,  h,  b);
    }
}

static void gameover_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    aux_proj_identity();
    aux_draw_tex(gameover_o, 0.0, 0.25, 1.0, 0.75, 1.0);
}

static int gameover_timer(double dt)
{
    if (time_state() > 3.0)
        goto_state(&st_title);

    return 0;
}

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

#define TITLE_S "data/png/title.png"

static void  *title_p = NULL;
static int    title_w = 0;
static int    title_h = 0;
static int    title_b = 0;
static GLuint title_o = 0;

static void title_enter(void)
{
    if (title_p == NULL)
    {
        title_p = aux_load_png(TITLE_S, &title_w, &title_h, &title_b);
        title_o = aux_make_tex(title_p,  title_w,  title_h,  title_b);
    }
}

static void title_leave(void)
{
    level_init();
    game_start();
}

static void title_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    aux_proj_identity();
    aux_draw_tex(title_o, 0.0, 0.25, 1.0, 0.75, 1.0);
}

static int title_timer(double dt)
{
    return 0;
}

static int title_point(int x, int y)
{
    return 0;
}

static int title_click(int d)
{
    return 0;
}

static int title_keybd(int c)
{
    if (c == 27)
        return -1;
    if (c == 32)
    {
        goto_state(&st_level);
        return +1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

struct state st_gameover = {
    gameover_enter,
    NULL,
    gameover_paint,
    gameover_timer,
    NULL,
    NULL,
    NULL
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
    title_timer,
    title_point,
    title_click,
    title_keybd
};
