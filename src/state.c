/*   
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "main.h"
#include "game.h"
#include "menu.h"
#include "text.h"
#include "state.h"
#include "level.h"
#include "image.h"
#include "audio.h"

/*---------------------------------------------------------------------------*/

static double goto_time    = 0.0;
static struct state *state = NULL;

int goto_state(struct state *st)
{
    if (state && state->leave)
        state->leave();

    state     = st;
    goto_time = SDL_GetTicks() / 1000.0;

    if (state && state->enter)
        state->enter();

    return 1;
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

int st_point(int x, int y, int dx, int dy)
{
    return (state && state->point) ? state->point(x, y, dx, dy) : 1;
}

int st_click(int b, int d)
{
    return (state && state->click) ? state->click(b, d) : 1;
}

int st_keybd(int c)
{
    return (state && state->keybd) ? state->keybd(c) : 1;
}

int st_stick(int a, int k)
{
    return (state && state->stick) ? state->stick(a, k) : 1;
}

int st_buttn(int b, int d)
{
    return (state && state->buttn) ? state->buttn(b, d) : 1;
}

/*---------------------------------------------------------------------------*/

#define TITLE_PLAY 1
#define TITLE_CONF 2
#define TITLE_EXIT 3

#define STR_TITLE " Neverball "
#define STR_PLAY  " Play "
#define STR_CONF  " Options "
#define STR_EXIT  " Exit "

static struct menu title_menu;

static int title_action(int i)
{
    switch (i)
    {
    case TITLE_PLAY: audio_play(AUD_MENU, 1.f); goto_state(&st_start); break;
    case TITLE_CONF: audio_play(AUD_MENU, 1.f); goto_state(&st_conf);  break;
    case TITLE_EXIT: return 0;
    }
    return 1;
}

static void title_enter(void)
{
    struct menu *p = &title_menu;

    const GLfloat c0[3] = { 1.0f, 1.0f, 0.0f };
    const GLfloat c1[3] = { 1.0f, 0.5f, 0.0f };
    const GLfloat c2[3] = { 1.0f, 1.0f, 1.0f };

    int lw, lh;
    int mw, mh;

    text_size(STR_TITLE, TXT_LRG, &lw, &lh);
    text_size(STR_CONF,  TXT_MED, &mw, &mh);

    menu_init(p, 4, 4, TITLE_PLAY);

    menu_text(p, 0,          0, mh + lh / 2, c0, c1, STR_TITLE, TXT_LRG);
    menu_text(p, TITLE_PLAY, 0, -0 * mh,     c2, c2, STR_PLAY,  TXT_MED);
    menu_text(p, TITLE_CONF, 0, -1 * mh,     c2, c2, STR_CONF,  TXT_MED);
    menu_text(p, TITLE_EXIT, 0, -2 * mh,     c2, c2, STR_EXIT,  TXT_MED);

    menu_item(p, 0,          0, mh + lh / 2, lw, lh);
    menu_item(p, TITLE_PLAY, 0, -0 * mh,     mw, mh);
    menu_item(p, TITLE_CONF, 0, -1 * mh,     mw, mh);
    menu_item(p, TITLE_EXIT, 0, -2 * mh,     mw, mh);

    menu_link(p, TITLE_PLAY, TITLE_EXIT, TITLE_CONF, -1, -1);
    menu_link(p, TITLE_CONF, TITLE_PLAY, TITLE_EXIT, -1, -1);
    menu_link(p, TITLE_EXIT, TITLE_CONF, TITLE_PLAY, -1, -1);

    menu_stat(p, 0,          -1);
    menu_stat(p, TITLE_PLAY, +1);
    menu_stat(p, TITLE_CONF,  0);
    menu_stat(p, TITLE_EXIT,  0);

    level_init();
    level_goto(0);
    audio_play(AUD_TITLE, 1.f);
}

static void title_leave(void)
{
    level_free();
    menu_free(&title_menu);
}

static void title_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();
    menu_paint(&title_menu, 1.0);
}

static int title_timer(double dt)
{
    game_update_fly(cos(time_state() / 10.0));
    return 1;
}

static int title_point(int x, int y, int dx, int dy)
{
    menu_point(&title_menu, x, y);
    return 1;
}

static int title_click(int b, int d)
{
    if (b < 0 && d == 1)
        return title_action(menu_click(&title_menu));
    return 1;
}

static int title_keybd(int c)
{
    if (c == SDLK_ESCAPE)
        return 0;
    return 1;
}

static int title_stick(int a, int v)
{
    menu_stick(&title_menu, a, v);
    return 1;
}

static int title_buttn(int b, int d)
{
    if (b == joy_button_a && d == 1)
        return title_action(menu_buttn(&title_menu));
    return 1;
}

/*---------------------------------------------------------------------------*/

#define CONF_FULL  0
#define CONF_WIN   1
#define CONF_16x12 2
#define CONF_12x10 3
#define CONF_10x7  4
#define CONF_8x6   5
#define CONF_6x4   6
#define CONF_TEXHI 7
#define CONF_TEXLO 8
#define CONF_GEOHI 9
#define CONF_GEOLO 10
#define CONF_AUDHI 11
#define CONF_AUDLO 12
#define CONF_BACK  13

static struct menu conf_menu;

static int conf_action(int i)
{
    int s = 1;

    switch (i)
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

    case CONF_16x12:
        goto_state(&st_null);
        s = set_mode(1600, 1200, main_mode);
        goto_state(&st_conf);
        break;

    case CONF_12x10:
        goto_state(&st_null);
        s = set_mode(1280, 1024, main_mode);
        goto_state(&st_conf);
        break;

    case CONF_10x7:
        goto_state(&st_null);
        s = set_mode(1024, 768, main_mode);
        goto_state(&st_conf);
        break;
            
    case CONF_8x6:
        goto_state(&st_null);
        s = set_mode(800, 600, main_mode);
        goto_state(&st_conf);
        break;

    case CONF_6x4:
        goto_state(&st_null);
        s = set_mode(640, 480, main_mode);
        goto_state(&st_conf);
        break;

    case CONF_TEXHI:
        goto_state(&st_null);
        set_image_scale(1);
        goto_state(&st_conf);
        break;

    case CONF_TEXLO:
        goto_state(&st_null);
        set_image_scale(2);
        goto_state(&st_conf);
        break;

    case CONF_GEOHI:
        goto_state(&st_null);
        main_geom = 1;
        goto_state(&st_conf);
        break;

    case CONF_GEOLO:
        goto_state(&st_null);
        main_geom = 0;
        goto_state(&st_conf);
        break;

    case CONF_AUDHI:
        audio_free();
        audio_init((main_rate = 44100), (main_buff = AUD_BUFF_HI));
        goto_state(&st_conf);
        break;

    case CONF_AUDLO:
        audio_free();
        audio_init((main_rate = 22050), (main_buff = AUD_BUFF_LO));
        goto_state(&st_conf);
        break;

    case CONF_BACK:
        goto_state(&st_title);
        break;
    }
    return s;
}

static void conf_enter(void)
{
    static int value = CONF_FULL;

    struct menu *p = &conf_menu;

    const GLfloat c0[3] = { 1.0f, 1.0f, 1.0f };
    const GLfloat c1[3] = { 1.0f, 1.0f, 0.0f };
    const GLfloat c2[3] = { 1.0f, 0.5f, 0.0f };

    int w, w2, w4;
    int h, h2;
    
    /* Compute menu sizes from font sizes */

    text_size("MMMMMMMMM", TXT_SML, &w, &h);
    w2 = w / 2;
    w4 = w / 4;
    h2 = h / 2;

    menu_init(&conf_menu, 28, 19, value);

    /* Text elements */

    menu_text(p,  0, -w2, +h2 + 3 * h, c1, c2, "Options",    TXT_MED);
    menu_text(p,  1, -w2,      +2 * h, c0, c0, "Fullscreen", TXT_SML);
    menu_text(p,  2, -w2,      +1 * h, c0, c0, "Window",     TXT_SML);
    menu_text(p,  3, +w2 - w4, +4 * h, c0, c0, "1600",       TXT_SML);
    menu_text(p,  4, +w2 - w4, +3 * h, c0, c0, "1280",       TXT_SML);
    menu_text(p,  5, +w2 - w4, +2 * h, c0, c0, "1024",       TXT_SML);
    menu_text(p,  6, +w2 - w4, +1 * h, c0, c0,  "800",       TXT_SML);
    menu_text(p,  7, +w2 - w4,  0 * h, c0, c0,  "640",       TXT_SML);
    menu_text(p,  8, +w2,      +4 * h, c0, c0,    "x",       TXT_SML);
    menu_text(p,  9, +w2,      +3 * h, c0, c0,    "x",       TXT_SML);
    menu_text(p, 10, +w2,      +2 * h, c0, c0,    "x",       TXT_SML);
    menu_text(p, 11, +w2,      +1 * h, c0, c0,    "x",       TXT_SML);
    menu_text(p, 12, +w2,       0 * h, c0, c0,    "x",       TXT_SML);
    menu_text(p, 13, +w2 + w4, +4 * h, c0, c0, "1200",       TXT_SML);
    menu_text(p, 14, +w2 + w4, +3 * h, c0, c0, "1024",       TXT_SML);
    menu_text(p, 15, +w2 + w4, +2 * h, c0, c0,  "768",       TXT_SML);
    menu_text(p, 16, +w2 + w4, +1 * h, c0, c0,  "600",       TXT_SML);
    menu_text(p, 17, +w2 + w4,  0 * h, c0, c0,  "480",       TXT_SML);
    menu_text(p, 18, -w2,      -1 * h, c1, c2, "Textures",   TXT_SML);
    menu_text(p, 19, -w2,      -2 * h, c1, c2, "Geometry",   TXT_SML);
    menu_text(p, 20, -w2,      -3 * h, c1, c2, "Audio",      TXT_SML);
    menu_text(p, 21, +w2 - w4, -1 * h, c0, c0, "High",       TXT_SML);
    menu_text(p, 22, +w2 - w4, -2 * h, c0, c0, "High",       TXT_SML);
    menu_text(p, 23, +w2 - w4, -3 * h, c0, c0, "High",       TXT_SML);
    menu_text(p, 24, +w2 + w4, -1 * h, c0, c0, "Low",        TXT_SML);
    menu_text(p, 25, +w2 + w4, -2 * h, c0, c0, "Low",        TXT_SML);
    menu_text(p, 26, +w2 + w4, -3 * h, c0, c0, "Low",        TXT_SML);
    menu_text(p, 27, +w2,      -4 * h, c0, c0, "Back",       TXT_SML);

    /* Active items */

    menu_item(p, CONF_FULL,  -w2,      +2 * h, w,  h);
    menu_item(p, CONF_WIN,   -w2,      +1 * h, w,  h);
    menu_item(p, CONF_16x12, +w2,      +4 * h, w,  h);
    menu_item(p, CONF_12x10, +w2,      +3 * h, w,  h);
    menu_item(p, CONF_10x7,  +w2,      +2 * h, w,  h);
    menu_item(p, CONF_8x6,   +w2,      +1 * h, w,  h);
    menu_item(p, CONF_6x4,   +w2,      +0 * h, w,  h);
    menu_item(p, CONF_TEXHI, +w2 - w4, -1 * h, w2, h);
    menu_item(p, CONF_TEXLO, +w2 + w4, -1 * h, w2, h);
    menu_item(p, CONF_GEOHI, +w2 - w4, -2 * h, w2, h);
    menu_item(p, CONF_GEOLO, +w2 + w4, -2 * h, w2, h);
    menu_item(p, CONF_AUDHI, +w2 - w4, -3 * h, w2, h);
    menu_item(p, CONF_AUDLO, +w2 + w4, -3 * h, w2, h);
    menu_item(p, CONF_BACK,  +w2,      -4 * h, w,  h);

    /* Inactive label padding */

    menu_item(p, 14, -w2, +4 * h, w,  h);
    menu_item(p, 15, -w2, +3 * h, w,  h);
    menu_item(p, 16, -w2, -1 * h, w,  h);
    menu_item(p, 17, -w2, -2 * h, w,  h);
    menu_item(p, 18, -w2, -3 * h, w,  h);

    /* Item state */

    menu_stat(p, CONF_FULL,  (main_mode & SDL_FULLSCREEN) ? 1 : 0);
    menu_stat(p, CONF_WIN,   (main_mode & SDL_FULLSCREEN) ? 0 : 1);
    menu_stat(p, CONF_16x12, (main_width == 1600)         ? 1 : 0);
    menu_stat(p, CONF_12x10, (main_width == 1280)         ? 1 : 0);
    menu_stat(p, CONF_10x7,  (main_width == 1024)         ? 1 : 0);
    menu_stat(p, CONF_8x6,   (main_width ==  800)         ? 1 : 0);
    menu_stat(p, CONF_6x4,   (main_width ==  640)         ? 1 : 0);
    menu_stat(p, CONF_TEXHI, (get_image_scale() == 1)     ? 1 : 0);
    menu_stat(p, CONF_TEXLO, (get_image_scale() == 2)     ? 1 : 0);
    menu_stat(p, CONF_GEOHI, (main_geom == 1)             ? 1 : 0);
    menu_stat(p, CONF_GEOLO, (main_geom == 0)             ? 1 : 0);
    menu_stat(p, CONF_AUDHI, (main_rate == 44100)         ? 1 : 0);
    menu_stat(p, CONF_AUDLO, (main_rate == 22050)         ? 1 : 0);
    menu_stat(p, CONF_BACK, 0);

    /* Item linkings for joystick menu traversal */

    menu_link(p, CONF_FULL,  CONF_FULL,  CONF_WIN,   CONF_FULL,  CONF_10x7);
    menu_link(p, CONF_WIN,   CONF_FULL,  CONF_WIN,   CONF_WIN,   CONF_8x6);
    menu_link(p, CONF_16x12, CONF_16x12, CONF_12x10, CONF_16x12, CONF_16x12);
    menu_link(p, CONF_12x10, CONF_16x12, CONF_10x7,  CONF_12x10, CONF_12x10);
    menu_link(p, CONF_10x7,  CONF_12x10, CONF_8x6,   CONF_FULL,  CONF_10x7);
    menu_link(p, CONF_8x6,   CONF_10x7,  CONF_6x4,   CONF_WIN,   CONF_8x6);
    menu_link(p, CONF_6x4,   CONF_8x6,   CONF_TEXHI, CONF_6x4,   CONF_6x4);
    menu_link(p, CONF_TEXHI, CONF_6x4,   CONF_GEOHI, CONF_TEXHI, CONF_TEXLO);
    menu_link(p, CONF_TEXLO, CONF_6x4,   CONF_GEOLO, CONF_TEXHI, CONF_TEXLO);
    menu_link(p, CONF_GEOHI, CONF_TEXHI, CONF_AUDHI, CONF_GEOHI, CONF_GEOLO);
    menu_link(p, CONF_GEOLO, CONF_TEXLO, CONF_AUDLO, CONF_GEOHI, CONF_GEOLO);
    menu_link(p, CONF_AUDHI, CONF_GEOHI, CONF_BACK,  CONF_AUDHI, CONF_AUDLO);
    menu_link(p, CONF_AUDLO, CONF_GEOLO, CONF_BACK,  CONF_AUDHI, CONF_AUDLO);
    menu_link(p, CONF_BACK,  CONF_AUDHI, CONF_BACK,  CONF_BACK,  CONF_BACK);

    value = -1;
}

static void conf_leave(void)
{
    conf_store();
    menu_free(&conf_menu);
}

static void conf_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    menu_paint(&conf_menu, 1.0);
}

static int conf_point(int x, int y, int dx, int dy)
{
    menu_point(&conf_menu, x, y);
    return 1;
}

static int conf_click(int b, int d)
{
    if (b < 0 && d == 1)
        return conf_action(menu_click(&conf_menu));

    return 1;
}

static int conf_keybd(int c)
{
    if (c == SDLK_ESCAPE)
        return goto_state(&st_title);

    return 1;
}

static int conf_stick(int a, int v)
{
    menu_stick(&conf_menu, a, v);
    return 1;
}

static int conf_buttn(int b, int d)
{
    if (b == joy_button_a && d == 1)
        return conf_action(menu_buttn(&conf_menu));
    if (b == joy_button_b && d == 1)
        return goto_state(&st_title);

    return 1;
}

/*---------------------------------------------------------------------------*/

#define START_BACK 0

#define STR_START "Level Select"
#define STR_BACK  "Back"

static struct menu start_menu;

static int start_action(int i)
{
    if (i >= 0)
    {
        if (i == START_BACK)
            return goto_state(&st_title);
        else
        {
            level_goto(i);
            return goto_state(&st_level);
        }
    }
    return 1;
}

static void start_enter(void)
{
    struct menu *p = &start_menu;

    const GLfloat c0[3] = { 1.0f, 1.0f, 1.0f };
    const GLfloat c1[3] = { 1.0f, 1.0f, 0.0f };
    const GLfloat c2[3] = { 1.0f, 0.5f, 0.0f };

    char str[3] = { '0', '0', 0 };
    int x, w, dx;
    int y, h, dy;
    int i = 1;

    level_init();
    menu_init(p, 66, 66, 0);

    text_size("00", TXT_SML, &w,  &h);
    dx = 3 * w / 2;
    dy = 3 * h / 2;

    /* Create level number text and items. */

    for (y = -4 * dy; y < +4 * dy; y += dy)
        for (x = -4 * dx; x < +4 * dx; x += dx)
        {
            int xc = x + dx / 2;
            int yc = y + dy / 2;
            int b  = (i < count);

            str[0] = '0' + i / 10;
            str[1] = '0' + i % 10;

            menu_text(p, i, xc, yc, b ? c0 : c1, b ? c0 : c2, str, TXT_SML);
            menu_item(p, i, xc, yc, dx, dy);
            menu_stat(p, i, i < count ? 0 : -1);
            menu_link(p, i,
                      (i + 8) < count ? (i + 8) : i,
                      (i - 8) >= 0    ? (i - 8) : START_BACK,
                      (i - 1) >= 0    ? (i - 1) : START_BACK,
                      (i + 1) < count ? (i + 1) : i);
            i++;
        }

    /* Create label and back button. */

    menu_text(p, 0,  -3 * dx, -5 * dy + dy / 2, c0, c0, STR_BACK,  TXT_SML);
    menu_item(p, 0,  -3 * dx, -5 * dy + dy / 2, 2 * dx, dy);
    menu_stat(p, 0, 0);
    menu_link(p, 0, 1, START_BACK, START_BACK, START_BACK);

    menu_text(p, 65, +2 * dx, +4 * dy + dy / 2, c1, c2, STR_START, TXT_SML);
    menu_item(p, 65, +2 * dx, +4 * dy + dy / 2, 4 * dx, dy);
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

static int start_point(int x, int y, int dx, int dy)
{
    menu_point(&start_menu, x, y);
    return 1;
}

static int start_click(int b, int d)
{
    if (b < 0 && d == 1)
        return start_action(menu_click(&start_menu));

    return 1;
}

static int start_keybd(int c)
{
    if (c == SDLK_ESCAPE)
        return goto_state(&st_title);

    return 1;
}

static int start_stick(int a, int v)
{
    menu_stick(&start_menu, a, v);
    return 1;
}

static int start_buttn(int b, int d)
{
    if (b == joy_button_a && d == 1)
        return start_action(menu_buttn(&start_menu));
    if (b == joy_button_b && d == 1)
        return goto_state(&st_title);

    return 1;
}

/*---------------------------------------------------------------------------*/

static struct menu level_menu;

static void level_enter(void)
{
    const GLfloat c0[3] = { 1.0f, 1.0f, 1.0f };
    const GLfloat c1[3] = { 1.0f, 1.0f, 0.0f };
    const GLfloat c2[3] = { 1.0f, 0.5f, 0.0f };

    int y = 1 * main_height / 6;
    int h = 1 * main_width  / 6;
    int w = 7 * main_width  / 8;
    int i, j, l;

    char buf[256], *p = game_note();

    sprintf(buf, "Level %02d", level);
    
    menu_init(&level_menu, 7, 7, 10);
    menu_text(&level_menu, 0, 0, y, c1, c2, buf, TXT_LRG);
    menu_item(&level_menu, 0, 0, y, w, h);

    /* Position the level message text. */

    text_size("M", TXT_SML, NULL, &l);
    y = 0;
    i = 1;

    while (p && i < 7)
    {
        memset(buf, 0, 256);
        j = 0;

        while (p && *p && *p != '\\')
            buf[j++] = *p++;

        if (strlen(buf) > 0)
            menu_text(&level_menu, i, 0, y, c0, c0, buf, TXT_SML);

        menu_item(&level_menu, i, 0, y, w, l);
        menu_stat(&level_menu, i, -1);

        y -= l;
        i++;
        p++;
    }

    audio_play(AUD_LEVEL, 1.f);
    game_update_fly(1.0);
}

static void level_leave(void)
{
    menu_free(&level_menu);
}

static void level_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();
    game_render_hud();
    menu_paint(&level_menu, 1.0);
}

static int level_click(int b, int d)
{
    if (b < 0 && d == 1)
        goto_state(&st_ready);

    return 1;
}

static int level_keybd(int c)
{
    if (c == SDLK_ESCAPE)
        goto_state(&st_over);

    return 1;
}

static int level_buttn(int b, int d)
{
    if (b == joy_button_a && d == 1)
        goto_state(&st_ready);

    return 1;
}

/*---------------------------------------------------------------------------*/

static struct menu ready_menu;

static void ready_enter(void)
{
    const GLfloat c0[3] = { 1.0f, 1.0f, 0.0f };
    const GLfloat c1[3] = { 1.0f, 0.5f, 0.0f };

    int y = 1 * main_height / 6;
    int h = 1 * main_width  / 6;
    int w = 2 * main_width  / 3;

    menu_init(&ready_menu, 1, 1, 1);
    menu_text(&ready_menu, 0, 0, y, c0, c1, "Ready?", TXT_LRG);
    menu_item(&ready_menu, 0, 0, y, w, h);

    audio_play(AUD_READY, 1.f);
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

    game_update_fly(1.0 - 0.5 * t);

    if (dt > 0.0 && t > 1.0)
        goto_state(&st_set);

    return 1;
}

static int ready_click(int b, int d)
{
    if (b < 0 && d == 1)
    {
        game_update_fly(0.0);
        goto_state(&st_play);
    }
    return 1;
}

static int ready_keybd(int c)
{
    if (c == SDLK_ESCAPE)
        goto_state(&st_over);

    return 1;
}

static int ready_buttn(int b, int d)
{
    if (b == joy_button_a && d == 1)
    {
        game_update_fly(0.0);
        goto_state(&st_play);
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

static struct menu set_menu;

static void set_enter(void)
{
    const GLfloat c0[3] = { 1.0f, 1.0f, 0.0f };
    const GLfloat c1[3] = { 1.0f, 0.5f, 0.0f };

    int y = 1 * main_height / 6;
    int h = 1 * main_width  / 6;
    int w = 2 * main_width  / 3;

    menu_init(&set_menu, 1, 1, 1);
    menu_text(&set_menu, 0, 0, y, c0, c1, "Set?", TXT_LRG);
    menu_item(&set_menu, 0, 0, y, w, h);

    audio_play(AUD_SET, 1.f);
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

    game_update_fly(0.5 - 0.5 * t);

    if (dt > 0.0 && t > 1.0)
        goto_state(&st_play);

    return 1;
}

static int set_click(int b, int d)
{
    if (b < 0 && d == 1)
    {
        game_update_fly(0.0);
        goto_state(&st_play);
    }
    return 1;
}

static int set_keybd(int c)
{
    if (c == SDLK_ESCAPE)
        goto_state(&st_over);

    return 1;
}

static int set_buttn(int b, int d)
{
    if (b == joy_button_a && d == 1)
    {
        game_update_fly(0.0);
        goto_state(&st_play);
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

static struct menu play_menu;
static int         view_rotate = 0;

static void play_enter(void)
{
    const GLfloat c0[3] = { 0.0f, 0.5f, 0.0f };
    const GLfloat c1[3] = { 0.0f, 1.0f, 0.0f };

    int y = 1 * main_height / 6;
    int h = 1 * main_width  / 6;
    int w = 2 * main_width  / 3;

    menu_init(&play_menu, 1, 1, 1);
    menu_text(&play_menu, 0, 0, y, c0, c1, "GO!", TXT_LRG);
    menu_item(&play_menu, 0, 0, y, w, h);

    audio_play(AUD_GO, 1.f);

    view_rotate = 0;

    SDL_ShowCursor(SDL_DISABLE);
    SDL_WM_GrabInput(SDL_GRAB_ON);
}

static void play_leave(void)
{
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_ShowCursor(SDL_ENABLE);

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

    game_update_rot(view_rotate);

    switch (game_update_env(g, dt))
    {
    case GAME_TIME: goto_state(&st_time); break;
    case GAME_GOAL: goto_state(&st_goal); break;
    case GAME_FALL: goto_state(&st_fall); break;
    }

    return 1;
}

static int play_point(int x, int y, int dx, int dy)
{
    /*
     * HACK: SDL's  relative mouse motion sometimes  makes large jumps
     * when you fiddle with the  grab while the pointer is outside the
     * window.  When a level begins, this can suddenly throw the floor
     * to a random angle.  So, we discard all big jumps.
     */

    if (abs(dx) < 50 && abs(dy) < 50)
    {
        game_update_pos(dx, dy);
    }
    return 1;
}

static int play_click(int b, int d)
{
    if (d)
        view_rotate = b;
    else
        view_rotate = 0;

    return 1;
}

static int play_keybd(int c)
{
    if (c == SDLK_ESCAPE)
        goto_state(&st_over);
    if (c == SDLK_SPACE)
        goto_state(&st_pause);

    return 1;
}

static int play_stick(int a, int k)
{
    if (a == joy_axis_x) game_update_z(k);
    if (a == joy_axis_y) game_update_x(k);

    return 1;
}

static int play_buttn(int b, int d)
{
    if (b == joy_button_r)
    {
        if (d == 1)
            view_rotate += 1;
        else
            view_rotate -= 1;
    }
    if (b == joy_button_l)
    {
        if (d == 1)
            view_rotate -= 1;
        else
            view_rotate += 1;
    }
    if (b == joy_button_pause && d == 1)
        goto_state(&st_pause);

    return 1;
}

/*---------------------------------------------------------------------------*/

static struct menu goal_menu;

static void goal_enter(void)
{
    const GLfloat c0[3] = { 0.0f, 0.5f, 0.0f };
    const GLfloat c1[3] = { 0.0f, 1.0f, 0.0f };

    int y = 1 * main_height / 6;
    int h = 1 * main_width  / 6;
    int w = 2 * main_width  / 3;

    menu_init(&goal_menu, 1, 1, 1);
    menu_text(&goal_menu, 0, 0, y, c0, c1, "GOAL!", TXT_LRG);
    menu_item(&goal_menu, 0, 0, y, w, h);

    audio_play(AUD_GOAL, 1.f);
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

static int goal_click(int b, int d)
{
    if (b <= 0 && d == 1)
    {
        if (level_pass())
            goto_state(&st_level);
        else
            goto_state(&st_over);
    }
    return 1;
}

static int goal_timer(double dt)
{
    double g[3] = { 0.0, 9.8, 0.0 };

    if (time_state() < 2.0)
        game_update_env(g, dt);
    else
        return goal_click(0, 1);

    return 1;
}

static int goal_buttn(int b, int d)
{
    if (b == joy_button_a && d == 1)
        return goal_click(0, 1);

    return 1;
}

/*---------------------------------------------------------------------------*/

static struct menu fall_menu;

static void fall_enter(void)
{
    const GLfloat c0[3] = { 0.5f, 0.0f, 0.0f };
    const GLfloat c1[3] = { 1.0f, 0.0f, 0.0f };

    int y = 1 * main_height / 6;
    int h = 1 * main_width  / 6;
    int w = 7 * main_width  / 8;

    menu_init(&fall_menu, 1, 1, 1);
    menu_text(&fall_menu, 0, 0, y, c0, c1, "Fall-out!", TXT_LRG);
    menu_item(&fall_menu, 0, 0, y, w, h);

    audio_play(AUD_FALL, 1.f);
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

static int fall_click(int b, int d)
{
    if (b <= 0 && d == 1)
    {
        if (level_fail())
            goto_state(&st_level);
        else
            goto_state(&st_over);
    }
    return 1;
}

static int fall_timer(double dt)
{
    double g[3] = { 0.0, -9.8, 0.0 };

    if (time_state() < 2.0)
        game_update_env(g, dt);
    else
        return fall_click(0, 1);

    return 1;
}

static int fall_buttn(int b, int d)
{
    if (b == joy_button_a && d == 1)
        return fall_click(0, 1);

    return 1;
}

/*---------------------------------------------------------------------------*/

static struct menu time_menu;

static void time_enter(void)
{
    const GLfloat c0[3] = { 0.5f, 0.0f, 0.0f };
    const GLfloat c1[3] = { 1.0f, 0.0f, 0.0f };

    int y = 1 * main_height / 6;
    int h = 1 * main_width  / 6;
    int w = 7 * main_width  / 8;

    menu_init(&time_menu, 1, 1, 1);
    menu_text(&time_menu, 0, 0, y, c0, c1, "Time's up!", TXT_LRG);
    menu_item(&time_menu, 0, 0, y, w, h);

    audio_play(AUD_TIME, 1.f);
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

static int time_click(int b, int d)
{
    if (b <= 0 && d == 1)
    {
        if (level_fail())
            goto_state(&st_level);
        else
            goto_state(&st_over);
    }
    return 1;
}

static int time_timer(double dt)
{
    double g[3] = { 0.0, -9.8, 0.0 };

    if (time_state() < 2.0)
        game_update_env(g, dt);
    else
        return time_click(0, 1);

    return 1;
}

static int time_buttn(int b, int d)
{
    if (b == joy_button_a && d == 1)
        return time_click(0, 1);

    return 1;
}

/*---------------------------------------------------------------------------*/

static struct menu over_menu;

static void over_enter(void)
{
    const GLfloat c0[3] = { 0.5f, 0.0f, 0.0f };
    const GLfloat c1[3] = { 1.0f, 0.0f, 0.0f };

    int y = 1 * main_height / 6;
    int h = 1 * main_width  / 6;
    int w = 7 * main_width  / 8;

    menu_init(&over_menu, 1, 1, 1);
    menu_text(&over_menu, 0, 0, y, c0, c1, "GAME OVER", TXT_LRG);
    menu_item(&over_menu, 0, 0, y, w, h);

    audio_play(AUD_OVER, 1.f);
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
    if (c == SDLK_ESCAPE)
        goto_state(&st_title);

    return 1;
}

static int over_click(int b, int d)
{
    if (b < 0 && d == 1)
        goto_state(&st_title);

    return 1;
}

static int over_buttn(int b, int d)
{
    if (b == joy_button_a && d == 1) goto_state(&st_title);
    if (b == joy_button_b && d == 1) goto_state(&st_title);

    return 1;
}

/*---------------------------------------------------------------------------*/

static struct menu pause_menu;

static void pause_enter(void)
{
    const GLfloat c0[3] = { 1.0f, 1.0f, 0.0f };
    const GLfloat c1[3] = { 1.0f, 0.5f, 0.0f };

    int y = 1 * main_height / 6;
    int h = 1 * main_width  / 6;
    int w = 2 * main_width  / 3;

    menu_init(&pause_menu, 1, 1, 1);
    menu_text(&pause_menu, 0, 0, y, c0, c1, "Paused", TXT_LRG);
    menu_item(&pause_menu, 0, 0, y, w, h);

    audio_play(AUD_PAUSE, 1.f);
}

static void pause_leave(void)
{
    menu_free(&pause_menu);
}

static void pause_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_render_env();
    game_render_hud();
    menu_paint(&pause_menu, 0.5);
}

static int pause_keybd(int c)
{
    if (c == SDLK_ESCAPE) goto_state(&st_over);
    if (c == SDLK_SPACE)  goto_state(&st_play);

    return 1;
}

static int pause_buttn(int b, int d)
{
    if (b == joy_button_b     && d == 1) goto_state(&st_over);
    if (b == joy_button_pause && d == 1) goto_state(&st_play);

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
    NULL,
    NULL,
};

struct state st_title = {
    title_enter,
    title_leave,
    title_paint,
    title_timer,
    title_point,
    title_click,
    title_keybd,
    title_stick,
    title_buttn,
};

struct state st_start = {
    start_enter,
    start_leave,
    start_paint,
    NULL,
    start_point,
    start_click,
    start_keybd,
    start_stick,
    start_buttn,
};

struct state st_conf = {
    conf_enter,
    conf_leave,
    conf_paint,
    NULL,
    conf_point,
    conf_click,
    conf_keybd,
    conf_stick,
    conf_buttn,
};

struct state st_level = {
    level_enter,
    level_leave,
    level_paint,
    NULL,
    NULL,
    level_click,
    level_keybd,
    NULL,
    level_buttn,
};

struct state st_ready = {
    ready_enter,
    ready_leave,
    ready_paint,
    ready_timer,
    NULL,
    ready_click,
    ready_keybd,
    NULL,
    ready_buttn,
};

struct state st_set = {
    set_enter,
    set_leave,
    set_paint,
    set_timer,
    NULL,
    set_click,
    set_keybd,
    NULL,
    set_buttn,
};

struct state st_play = {
    play_enter,
    play_leave,
    play_paint,
    play_timer,
    play_point,
    play_click,
    play_keybd,
    play_stick,
    play_buttn,
};

struct state st_goal = {
    goal_enter,
    goal_leave,
    goal_paint,
    goal_timer,
    NULL,
    goal_click,
    NULL,
    NULL,
    goal_buttn,
};

struct state st_fall = {
    fall_enter,
    fall_leave,
    fall_paint,
    fall_timer,
    NULL,
    fall_click,
    NULL,
    NULL,
    fall_buttn,
};

struct state st_time = {
    time_enter,
    time_leave,
    time_paint,
    time_timer,
    NULL,
    time_click,
    NULL,
    NULL,
    time_buttn,
};

struct state st_over = {
    over_enter,
    over_leave,
    over_paint,
    over_timer,
    NULL,
    over_click,
    over_keybd,
    NULL,
    over_buttn,
};

struct state st_pause = {
    pause_enter,
    pause_leave,
    pause_paint,
    NULL,
    NULL,
    NULL,
    pause_keybd,
    NULL,
    pause_buttn,
};

