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

#include "glext.h"
#include "hud.h"
#include "game.h"
#include "menu.h"
#include "text.h"
#include "state.h"
#include "level.h"
#include "image.h"
#include "audio.h"
#include "config.h"

/*---------------------------------------------------------------------------*/

static double goto_time    = 0.0;
static struct state *state = NULL;

void init_state(void)
{
    state = &st_title;

    if (state->enter)
        state->enter();
}

double time_state(void)
{
    return SDL_GetTicks() / 1000.0 - goto_time;
}

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
    int lw, lh, lm;
    int mw, mh;

    text_size(STR_TITLE, TXT_LRG, &lw, &lh);
    text_size(STR_CONF,  TXT_MED, &mw, &mh);

    lm = lh / 2;

    menu_init(4, 4, TITLE_PLAY);

    menu_text(0,          0, mh + lm, c_yellow, c_red,   STR_TITLE, TXT_LRG);
    menu_text(TITLE_PLAY, 0, -0 * mh, c_white,  c_white, STR_PLAY,  TXT_MED);
    menu_text(TITLE_CONF, 0, -1 * mh, c_white,  c_white, STR_CONF,  TXT_MED);
    menu_text(TITLE_EXIT, 0, -2 * mh, c_white,  c_white, STR_EXIT,  TXT_MED);

    menu_item(0,          0, mh + lm, lw, lh);
    menu_item(TITLE_PLAY, 0, -0 * mh, mw, mh);
    menu_item(TITLE_CONF, 0, -1 * mh, mw, mh);
    menu_item(TITLE_EXIT, 0, -2 * mh, mw, mh);

    menu_link(TITLE_PLAY,         -1, TITLE_CONF, -1, -1);
    menu_link(TITLE_CONF, TITLE_PLAY, TITLE_EXIT, -1, -1);
    menu_link(TITLE_EXIT, TITLE_CONF,         -1, -1, -1);

    menu_stat(0,          -1);
    menu_stat(TITLE_PLAY, +1);
    menu_stat(TITLE_CONF,  0);
    menu_stat(TITLE_EXIT,  0);

    level_init();
    level_goto(0);
    audio_play(AUD_TITLE, 1.f);
}

static void title_leave(void)
{
    level_free();
    menu_free();
}

static void title_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0);
    menu_paint();
}

static int title_timer(double dt)
{
    game_set_fly(cos(time_state() / 10.0));
    return 1;
}

static int title_point(int x, int y, int dx, int dy)
{
    menu_point(dx, dy);
    return 1;
}

static int title_click(int b, int d)
{
    return (b < 0 && d == 1) ? title_action(menu_click()) : 1;
}

static int title_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? 0 : 1;
}

static int title_stick(int a, int v)
{
    menu_stick(a, v);
    return 1;
}

static int title_buttn(int b, int d)
{
    return (config_button_a(b) && d == 1) ? title_action(menu_buttn()) : 1;
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

static int conf_action(int i)
{
    int s = 1;

    switch (i)
    {
    case CONF_FULL:
        goto_state(&st_null);
        s = config_set_mode(config_w(), config_h(), SDL_OPENGL|SDL_FULLSCREEN);
        goto_state(&st_conf);
        break;

    case CONF_WIN:
        goto_state(&st_null);
        s = config_set_mode(config_w(), config_h(), SDL_OPENGL);
        goto_state(&st_conf);
        break;

    case CONF_16x12:
        goto_state(&st_null);
        s = config_set_mode(1600, 1200, config_mode());
        goto_state(&st_conf);
        break;

    case CONF_12x10:
        goto_state(&st_null);
        s = config_set_mode(1280, 1024, config_mode());
        goto_state(&st_conf);
        break;

    case CONF_10x7:
        goto_state(&st_null);
        s = config_set_mode(1024, 768, config_mode());
        goto_state(&st_conf);
        break;
            
    case CONF_8x6:
        goto_state(&st_null);
        s = config_set_mode(800, 600, config_mode());
        goto_state(&st_conf);
        break;

    case CONF_6x4:
        goto_state(&st_null);
        s = config_set_mode(640, 480, config_mode());
        goto_state(&st_conf);
        break;

    case CONF_TEXHI:
        goto_state(&st_null);
        config_set_text(1);
        goto_state(&st_conf);
        break;

    case CONF_TEXLO:
        goto_state(&st_null);
        config_set_text(2);
        goto_state(&st_conf);
        break;

    case CONF_GEOHI:
        goto_state(&st_null);
        config_set_geom(1);
        goto_state(&st_conf);
        break;

    case CONF_GEOLO:
        goto_state(&st_null);
        config_set_geom(0);
        goto_state(&st_conf);
        break;

    case CONF_AUDHI:
        s = config_set_audio(44100, AUD_BUFF_HI);
        goto_state(&st_conf);
        break;

    case CONF_AUDLO:
        s = config_set_audio(22050, AUD_BUFF_LO);
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

    int w, w2, w4;
    int h, h2;
    
    /* Compute menu sizes from font sizes */

    text_size("MMMMMMMMM", TXT_SML, &w, &h);
    w2 = w / 2;
    w4 = w / 4;
    h2 = h / 2;

    menu_init(28, 19, value);

    /* Text elements */

    menu_text(0,  -w2, +h2 + 3 * h, c_yellow, c_red,   "Options",    TXT_MED);
    menu_text(1,  -w2,      +2 * h, c_white,  c_white, "Fullscreen", TXT_SML);
    menu_text(2,  -w2,      +1 * h, c_white,  c_white, "Window",     TXT_SML);
    menu_text(3,  +w2 - w4, +4 * h, c_white,  c_white, "1600",       TXT_SML);
    menu_text(4,  +w2 - w4, +3 * h, c_white,  c_white, "1280",       TXT_SML);
    menu_text(5,  +w2 - w4, +2 * h, c_white,  c_white, "1024",       TXT_SML);
    menu_text(6,  +w2 - w4, +1 * h, c_white,  c_white,  "800",       TXT_SML);
    menu_text(7,  +w2 - w4,  0 * h, c_white,  c_white,  "640",       TXT_SML);
    menu_text(8,  +w2,      +4 * h, c_white,  c_white,    "x",       TXT_SML);
    menu_text(9,  +w2,      +3 * h, c_white,  c_white,    "x",       TXT_SML);
    menu_text(10, +w2,      +2 * h, c_white,  c_white,    "x",       TXT_SML);
    menu_text(11, +w2,      +1 * h, c_white,  c_white,    "x",       TXT_SML);
    menu_text(12, +w2,       0 * h, c_white,  c_white,    "x",       TXT_SML);
    menu_text(13, +w2 + w4, +4 * h, c_white,  c_white, "1200",       TXT_SML);
    menu_text(14, +w2 + w4, +3 * h, c_white,  c_white, "1024",       TXT_SML);
    menu_text(15, +w2 + w4, +2 * h, c_white,  c_white,  "768",       TXT_SML);
    menu_text(16, +w2 + w4, +1 * h, c_white,  c_white,  "600",       TXT_SML);
    menu_text(17, +w2 + w4,  0 * h, c_white,  c_white,  "480",       TXT_SML);
    menu_text(18, -w2,      -1 * h, c_yellow, c_red,   "Textures",   TXT_SML);
    menu_text(19, -w2,      -2 * h, c_yellow, c_red,   "Geometry",   TXT_SML);
    menu_text(20, -w2,      -3 * h, c_yellow, c_red,   "Audio",      TXT_SML);
    menu_text(21, +w2 - w4, -1 * h, c_white,  c_white, "High",       TXT_SML);
    menu_text(22, +w2 - w4, -2 * h, c_white,  c_white, "High",       TXT_SML);
    menu_text(23, +w2 - w4, -3 * h, c_white,  c_white, "High",       TXT_SML);
    menu_text(24, +w2 + w4, -1 * h, c_white,  c_white, "Low",        TXT_SML);
    menu_text(25, +w2 + w4, -2 * h, c_white,  c_white, "Low",        TXT_SML);
    menu_text(26, +w2 + w4, -3 * h, c_white,  c_white, "Low",        TXT_SML);
    menu_text(27, +w2,      -4 * h, c_white,  c_white, "Back",       TXT_SML);

    /* Active items */

    menu_item(CONF_FULL,  -w2,      +2 * h, w,  h);
    menu_item(CONF_WIN,   -w2,      +1 * h, w,  h);
    menu_item(CONF_16x12, +w2,      +4 * h, w,  h);
    menu_item(CONF_12x10, +w2,      +3 * h, w,  h);
    menu_item(CONF_10x7,  +w2,      +2 * h, w,  h);
    menu_item(CONF_8x6,   +w2,      +1 * h, w,  h);
    menu_item(CONF_6x4,   +w2,      +0 * h, w,  h);
    menu_item(CONF_TEXHI, +w2 - w4, -1 * h, w2, h);
    menu_item(CONF_TEXLO, +w2 + w4, -1 * h, w2, h);
    menu_item(CONF_GEOHI, +w2 - w4, -2 * h, w2, h);
    menu_item(CONF_GEOLO, +w2 + w4, -2 * h, w2, h);
    menu_item(CONF_AUDHI, +w2 - w4, -3 * h, w2, h);
    menu_item(CONF_AUDLO, +w2 + w4, -3 * h, w2, h);
    menu_item(CONF_BACK,  +w2,      -4 * h, w,  h);

    /* Inactive label padding */

    menu_item(14, -w2, +4 * h, w,  h);
    menu_item(15, -w2, +3 * h, w,  h);
    menu_item(16, -w2, -1 * h, w,  h);
    menu_item(17, -w2, -2 * h, w,  h);
    menu_item(18, -w2, -3 * h, w,  h);

    /* Item state */

    menu_stat(CONF_FULL,  (config_mode() & SDL_FULLSCREEN) ? 1 : 0);
    menu_stat(CONF_WIN,   (config_mode() & SDL_FULLSCREEN) ? 0 : 1);
    menu_stat(CONF_16x12, (config_w() == 1600)             ? 1 : 0);
    menu_stat(CONF_12x10, (config_w() == 1280)             ? 1 : 0);
    menu_stat(CONF_10x7,  (config_w() == 1024)             ? 1 : 0);
    menu_stat(CONF_8x6,   (config_w() ==  800)             ? 1 : 0);
    menu_stat(CONF_6x4,   (config_w() ==  640)             ? 1 : 0);
    menu_stat(CONF_TEXHI, (config_text() == 1)             ? 1 : 0);
    menu_stat(CONF_TEXLO, (config_text() == 2)             ? 1 : 0);
    menu_stat(CONF_GEOHI, (config_geom() == 1)             ? 1 : 0);
    menu_stat(CONF_GEOLO, (config_geom() == 0)             ? 1 : 0);
    menu_stat(CONF_AUDHI, (config_rate() == 44100)         ? 1 : 0);
    menu_stat(CONF_AUDLO, (config_rate() == 22050)         ? 1 : 0);
    menu_stat(CONF_BACK, 0);

    /* Item linkings for joystick menu traversal */

    menu_link(CONF_FULL,  CONF_FULL,  CONF_WIN,   CONF_FULL,  CONF_10x7);
    menu_link(CONF_WIN,   CONF_FULL,  CONF_WIN,   CONF_WIN,   CONF_8x6);
    menu_link(CONF_16x12, CONF_16x12, CONF_12x10, CONF_16x12, CONF_16x12);
    menu_link(CONF_12x10, CONF_16x12, CONF_10x7,  CONF_12x10, CONF_12x10);
    menu_link(CONF_10x7,  CONF_12x10, CONF_8x6,   CONF_FULL,  CONF_10x7);
    menu_link(CONF_8x6,   CONF_10x7,  CONF_6x4,   CONF_WIN,   CONF_8x6);
    menu_link(CONF_6x4,   CONF_8x6,   CONF_TEXHI, CONF_6x4,   CONF_6x4);
    menu_link(CONF_TEXHI, CONF_6x4,   CONF_GEOHI, CONF_TEXHI, CONF_TEXLO);
    menu_link(CONF_TEXLO, CONF_6x4,   CONF_GEOLO, CONF_TEXHI, CONF_TEXLO);
    menu_link(CONF_GEOHI, CONF_TEXHI, CONF_AUDHI, CONF_GEOHI, CONF_GEOLO);
    menu_link(CONF_GEOLO, CONF_TEXLO, CONF_AUDLO, CONF_GEOHI, CONF_GEOLO);
    menu_link(CONF_AUDHI, CONF_GEOHI, CONF_BACK,  CONF_AUDHI, CONF_AUDLO);
    menu_link(CONF_AUDLO, CONF_GEOLO, CONF_BACK,  CONF_AUDHI, CONF_AUDLO);
    menu_link(CONF_BACK,  CONF_AUDHI, CONF_BACK,  CONF_BACK,  CONF_BACK);

    value = -1;
}

static void conf_leave(void)
{
    config_store();
    menu_free();
}

static void conf_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    menu_paint();
}

static int conf_point(int x, int y, int dx, int dy)
{
    menu_point(dx, dy);
    return 1;
}

static int conf_click(int b, int d)
{
    return (b < 0 && d == 1) ? conf_action(menu_click()) : 1;
}

static int conf_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int conf_stick(int a, int v)
{
    menu_stick(a, v);
    return 1;
}

static int conf_buttn(int b, int d)
{
    if (config_button_a(b) && d == 1)
        return conf_action(menu_buttn());
    if (config_button_b(b) && d == 1)
        return goto_state(&st_title);
    return 1;
}

/*---------------------------------------------------------------------------*/

#define START_BACK 0

#define STR_START "Level Select"
#define STR_BACK  "Back"

static int shot_level = 0;
static int shot_x0    = 0;
static int shot_y0    = 0;
static int shot_x1    = 0;
static int shot_y1    = 0;

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
    int x, w, sw, dx, cx;
    int y, h, sh, dy;
    int i = 1;

    level_init();

    /* Compute the menu layout parameters based on text size. */

    text_size("00", TXT_SML, &w, &h);
    dx = 3 * w / 2;
    dy = 3 * h / 2;

    sw = 20 * dy / 3;
    sh =  5 * dy;
    cx = (5 * dx - sw) / 2;

    /* Create level number text and items. */

    menu_init(27, 27, 0);

    for (y = -2 * dy; y <= +2 * dy; y += dy)
        for (x = -9 * dx / 2; x < 0; x += dx)
        {
            char str[3];
            const float *c0 = c_black;
            const float *c1 = c_black;

            if (level_exists(i))
            {
                c0 = c_yellow;
                c1 = c_red;
            }
            if (level_opened(i))
            {
                c0 = c_white;
                c1 = c_white;
            }

            str[0] = '0' + i / 10;
            str[1] = '0' + i % 10;
            str[2] =  0;

            menu_text(i, cx + x, y, c0, c1, str, TXT_SML);
            menu_item(i, cx + x, y, dx, dy);
            menu_stat(i, level_opened(i) ? 0 : -1);
            menu_link(i,
                      level_opened(i + 5) ? (i + 5) : i,
                      level_opened(i - 5) ? (i - 5) : START_BACK,
                      level_opened(i - 1) ? (i - 1) : START_BACK,
                      level_opened(i + 1) ? (i + 1) : i);
            i++;
        }

    /* Create label and back button. */

    menu_text(0, cx -4*dx, -3*dy, c_white, c_white, STR_BACK, TXT_SML);
    menu_item(0, cx -4*dx, -3*dy, +2*dx, dy);
    menu_stat(0, 0);
    menu_link(0, 1, START_BACK, START_BACK, START_BACK);

    menu_text(26, cx + sw -2*dx, +3*dy, c_yellow, c_red, STR_START, TXT_SML);
    menu_item(26, cx + sw -2*dx, +3*dy, +4*dx, dy);

    /* Position the level shot. */

    shot_x0 =  (config_w() / 2) + cx;
    shot_y0 =  (config_h() / 2) - sh / 2;
    shot_x1 =  (config_w() / 2) + cx + sw;
    shot_y1 =  (config_h() / 2) + sh / 2;
}

static void start_leave(void)
{
    menu_free();
}

static void start_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    config_push_ortho();
    {
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        level_shot(shot_level);

        glBegin(GL_QUADS);
        {
            glColor3f(1.f, 1.f, 1.f);

            glTexCoord2i(0, 1); glVertex2i(shot_x0, shot_y0);
            glTexCoord2i(1, 1); glVertex2i(shot_x1, shot_y0);
            glTexCoord2i(1, 0); glVertex2i(shot_x1, shot_y1);
            glTexCoord2i(0, 0); glVertex2i(shot_x0, shot_y1);
        }
        glEnd();
    }
    config_pop_matrix();
    glPopAttrib();
    glPopAttrib();

    menu_paint();
}

static int start_point(int x, int y, int dx, int dy)
{
    int i;

    if ((i = menu_point(dx, dy)) >= 0)
        shot_level = i;

    return 1;
}

static int start_click(int b, int d)
{
    return (b < 0 && d == 1) ? start_action(menu_click()) : 1;
}

static int start_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int start_stick(int a, int v)
{
    int i;

    if ((i = menu_stick(a, v)) >= 0)
        shot_level = i;

    return 1;
}

static int start_buttn(int b, int d)
{
    if (config_button_a(b) && d == 1)
        return start_action(menu_buttn());
    if (config_button_b(b) && d == 1)
        return goto_state(&st_title);
    return 1;
}

/*---------------------------------------------------------------------------*/

static void level_enter(void)
{
    int y = 1 * config_h() / 6;
    int h = 1 * config_w() / 6;
    int w = 7 * config_w() / 8;
    int i, j, l = curr_level();

    char buf[256], *p = curr_intro();

    sprintf(buf, "Level %02d", l);
    
    menu_init(7, 7, 10);
    menu_text(0, 0, y, c_yellow, c_red, buf, TXT_LRG);
    menu_item(0, 0, y, w, h);

    /* Position the level intro message text. */

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
            menu_text(i, 0, y, c_white, c_white, buf, TXT_SML);

        menu_item(i, 0, y, w, l);
        menu_stat(i, -1);

        y -= l;
        i++;

        if (*p) p++;
    }

    audio_play(AUD_LEVEL, 1.f);
    game_set_fly(1.0);
}

static void level_leave(void)
{
    menu_free();
}

static void level_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0);
    hud_draw();
    menu_paint();
}

static int level_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_ready) : 1;
}

static int level_keybd(int c)
{
    if (c == SDLK_ESCAPE)
        goto_state(&st_over);
    if (c == SDLK_F12)
        goto_state(&st_poser);
    return 1;
}

static int level_buttn(int b, int d)
{
    return (config_button_a(b) && d == 1) ? goto_state(&st_ready) : 1;
}

/*---------------------------------------------------------------------------*/

static void poser_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(1);
}

static int poser_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_level) : 1;
}

/*---------------------------------------------------------------------------*/

static void ready_enter(void)
{
    int y = 1 * config_h() / 6;
    int h = 1 * config_w() / 6;
    int w = 2 * config_w() / 3;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_yellow, c_red, "Ready?", TXT_LRG);
    menu_item(0, 0, y, w, h);

    audio_play(AUD_READY, 1.f);
}

static void ready_leave(void)
{
    menu_free();
}

static void ready_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0);
    hud_draw();
    menu_paint();
}

static int ready_timer(double dt)
{
    double t = time_state();

    game_set_fly(1.0 - 0.5 * t);

    if (dt > 0.0 && t > 1.0)
        return goto_state(&st_set);

    return 1;
}

static int ready_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_play) : 1;
}

static int ready_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_over) : 1;
}

static int ready_buttn(int b, int d)
{
    return (config_button_a(b) && d == 1) ? goto_state(&st_play) : 1;
}

/*---------------------------------------------------------------------------*/

static void set_enter(void)
{
    int y = 1 * config_h() / 6;
    int h = 1 * config_w() / 6;
    int w = 2 * config_w() / 3;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_yellow, c_red, "Set?", TXT_LRG);
    menu_item(0, 0, y, w, h);

    audio_play(AUD_SET, 1.f);
}

static void set_leave(void)
{
    menu_free();
}

static void set_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0);
    hud_draw();
    menu_paint();
}

static int set_timer(double dt)
{
    double t = time_state();

    game_set_fly(0.5 - 0.5 * t);

    if (dt > 0.0 && t > 1.0)
        return goto_state(&st_play);

    return 1;
}

static int set_click(int b, int d)
{
    if (b < 0 && d == 1)
    {
        game_set_fly(0.0);
        return goto_state(&st_play);
    }
    return 1;
}

static int set_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_over) : 1;
}

static int set_buttn(int b, int d)
{
    return (config_button_a(b) && d == 1) ? goto_state(&st_play) : 1;
}

/*---------------------------------------------------------------------------*/

static int view_rotate = 0;

static void play_enter(void)
{
    int y = 1 * config_h() / 6;
    int h = 1 * config_w() / 6;
    int w = 2 * config_w() / 3;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_blue, c_green, "GO!", TXT_LRG);
    menu_item(0, 0, y, w, h);

    audio_play(AUD_GO, 1.f);

    game_set_fly(0.0);
    view_rotate = 0;
}

static void play_leave(void)
{
    menu_free();
}

static void play_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0);
    hud_draw();

    if (time_state() < 1.0)
        menu_paint();
}

static int play_timer(double dt)
{
    double g[3] = { 0.0, -9.8, 0.0 };

    game_set_rot(view_rotate);

    switch (game_step(g, dt))
    {
    case GAME_TIME: goto_state(&st_time); break;
    case GAME_GOAL: goto_state(&st_goal); break;
    case GAME_FALL: goto_state(&st_fall); break;
    }

    return 1;
}

static int play_point(int x, int y, int dx, int dy)
{
    game_set_pos(dx, dy);

    return 1;
}

static int play_click(int b, int d)
{
    view_rotate = d ? b : 0;
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
    if (config_axis_x(a)) game_set_z(k);
    if (config_axis_y(a)) game_set_x(k);
    return 1;
}

static int play_buttn(int b, int d)
{
    if (config_button_r(b))
    {
        if (d == 1)
            view_rotate += 1;
        else
            view_rotate -= 1;
    }
    if (config_button_l(b))
    {
        if (d == 1)
            view_rotate -= 1;
        else
            view_rotate += 1;
    }

    return (config_button_P(b) && d == 1) ? goto_state(&st_pause) : 1;
}

/*---------------------------------------------------------------------------*/

static void goal_enter(void)
{
    int y = 1 * config_h() / 6;
    int h = 1 * config_w() / 6;
    int w = 2 * config_w() / 3;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_blue, c_green, "GOAL!", TXT_LRG);
    menu_item(0, 0, y, w, h);

    audio_play(AUD_GOAL, 1.f);
}

static void goal_leave(void)
{
    menu_free();
}

static void goal_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0);
    menu_paint();
}

static int goal_click(int b, int d)
{
    if (b <= 0 && d == 1)
    {
        if (level_pass())
            goto_state(&st_level);
        else
            goto_state(&st_done);
    }
    return 1;
}

static int goal_timer(double dt)
{
    double g[3] = { 0.0, 9.8, 0.0 };

    if (time_state() < 2.0)
        game_step(g, dt);
    else
        return goal_click(0, 1);

    return 1;
}

static int goal_buttn(int b, int d)
{
    if (config_button_a(b) && d == 1)
        return goal_click(0, 1);
    return 1;
}

/*---------------------------------------------------------------------------*/

static void fall_enter(void)
{
    int y = 1 * config_h() / 6;
    int h = 1 * config_w() / 6;
    int w = 7 * config_w() / 8;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_black, c_red, "Fall-out!", TXT_LRG);
    menu_item(0, 0, y, w, h);

    audio_play(AUD_FALL, 1.f);
}

static void fall_leave(void)
{
    menu_free();
}

static void fall_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0);
    hud_draw();
    menu_paint();
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
        game_step(g, dt);
    else
        return fall_click(0, 1);

    return 1;
}

static int fall_buttn(int b, int d)
{
    return (config_button_a(b) && d == 1) ? fall_click(0, 1) : 1;
}

/*---------------------------------------------------------------------------*/

static void time_enter(void)
{
    int y = 1 * config_h() / 6;
    int h = 1 * config_w() / 6;
    int w = 7 * config_w() / 8;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_black, c_red, "Time's up!", TXT_LRG);
    menu_item(0, 0, y, w, h);

    audio_play(AUD_TIME, 1.f);
}

static void time_leave(void)
{
    menu_free();
}

static void time_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0);
    hud_draw();
    menu_paint();
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
        game_step(g, dt);
    else
        return time_click(0, 1);

    return 1;
}

static int time_buttn(int b, int d)
{
    return (config_button_a(b) && d == 1) ? time_click(0, 1) : 1;
}

/*---------------------------------------------------------------------------*/

static void over_enter(void)
{
    int y = 1 * config_h() / 6;
    int h = 1 * config_w() / 6;
    int w = 7 * config_w() / 8;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_black, c_red, "GAME OVER", TXT_LRG);
    menu_item(0, 0, y, w, h);

    audio_play(AUD_OVER, 1.f);
}

static void over_leave(void)
{
    menu_free();
    level_free();
}

static void over_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0);
    menu_paint();
}

static int over_timer(double dt)
{
    return (dt > 0.0 && time_state() > 3.0) ? goto_state(&st_title) : 1;
}

static int over_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int over_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_title) : 1;
}

static int over_buttn(int b, int d)
{
    if (config_button_a(b) && d == 1) goto_state(&st_title);
    if (config_button_b(b) && d == 1) goto_state(&st_title);
    return 1;
}

/*---------------------------------------------------------------------------*/

#define STR_DONE " That's it? "
#define STR_THNX " Thanks for playing! "
#define STR_MORE " More levels are on the way "

static void done_enter(void)
{
    int lw, lh, lm;
    int sw, sh;

    text_size(STR_DONE, TXT_LRG, &lw, &lh);
    text_size(STR_MORE, TXT_SML, &sw, &sh);

    lm = lh / 2;
    
    menu_init(3, 3, 10);
    menu_text(0, 0, sh + lm, c_green, c_yellow, STR_DONE, TXT_LRG);
    menu_text(1, 0, -0 * sh, c_white, c_white,  STR_THNX, TXT_SML);
    menu_text(2, 0, -1 * sh, c_white, c_white,  STR_MORE, TXT_SML);

    menu_item(0, 0, sh + lm, lw, lh);
    menu_item(1, 0, -0 * sh, sw, sh);
    menu_item(2, 0, -1 * sh, sw, sh);

    audio_play(AUD_OVER, 1.f);
}

static void done_leave(void)
{
    menu_free();
    level_free();
}

static void done_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0);
    menu_paint();
}

static int done_timer(double dt)
{
    return (dt > 0.0 && time_state() > 10.0) ? goto_state(&st_title) : 1;
}

static int done_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int done_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_title) : 1;
}

static int done_buttn(int b, int d)
{
    if (config_button_a(b) && d == 1) goto_state(&st_title);
    if (config_button_b(b) && d == 1) goto_state(&st_title);
    return 1;
}

/*---------------------------------------------------------------------------*/

static void pause_enter(void)
{
    int y = 1 * config_h() / 6;
    int h = 1 * config_w() / 6;
    int w = 2 * config_w() / 3;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_yellow, c_red, "Paused", TXT_LRG);
    menu_item(0, 0, y, w, h);

    audio_play(AUD_PAUSE, 1.f);

    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_ShowCursor(SDL_ENABLE);
}

static void pause_leave(void)
{
    SDL_ShowCursor(SDL_DISABLE);
    SDL_WM_GrabInput(SDL_GRAB_ON);

    menu_free();
}

static void pause_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0);
    hud_draw();
    menu_paint();
}

static int pause_keybd(int c)
{
    if (c == SDLK_ESCAPE) goto_state(&st_over);
    if (c == SDLK_SPACE)  goto_state(&st_play);
    return 1;
}

static int pause_buttn(int b, int d)
{
    if (config_button_b(b) && d == 1) goto_state(&st_over);
    if (config_button_P(b) && d == 1) goto_state(&st_play);
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

struct state st_poser = {
    NULL,
    NULL,
    poser_paint,
    NULL,
    NULL,
    NULL,
    poser_keybd,
    NULL,
    NULL,
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

struct state st_done = {
    done_enter,
    done_leave,
    done_paint,
    done_timer,
    NULL,
    done_click,
    done_keybd,
    NULL,
    done_buttn,
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

