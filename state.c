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

static double state_time   = 0.0;
static struct state *state = NULL;

void init_state(void)
{
    state = &st_title;

    if (state->enter)
        state->enter();
}

double time_state(void)
{
    return state_time;
}

int goto_state(struct state *st)
{
    if (state && state->leave)
        state->leave();

    state      = st;
    state_time =  0;

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
    state_time += t;
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

    SDL_ShowCursor(SDL_ENABLE);
}

static void title_leave(void)
{
    SDL_ShowCursor(SDL_DISABLE);
    
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
    menu_point(x, y);
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
    if (config_button_a(b) && d == 1) return title_action(menu_buttn());
    if (config_button_X(b) && d == 1) return 0;
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

    /* Item linkings for menu traversal */

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

    SDL_ShowCursor(SDL_ENABLE);
}

static void conf_leave(void)
{
    SDL_ShowCursor(SDL_DISABLE);

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
    menu_point(x, y);
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
    if (config_button_a(b) && d == 1) return conf_action(menu_buttn());
    if (config_button_b(b) && d == 1) return goto_state(&st_title);
    if (config_button_X(b) && d == 1) return goto_state(&st_title);
    return 1;
}

/*---------------------------------------------------------------------------*/

#define START_BACK 0

#define STR_START "Level Select"
#define STR_BACK  "Back"

static int shot_level = -1;
static int shot_x0    =  0;
static int shot_y0    =  0;
static int shot_x1    =  0;
static int shot_y1    =  0;

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

static void start_motion(int i)
{
    const GLfloat *c0 = c_yellow;
    const GLfloat *c1 = c_white;
    int w, h;

    if (shot_level != i)
    {
        shot_level = i;

        text_size("0", TXT_SML, &w, &h);

        menu_text(29, -11 * h, -15 * h/4, c0, c1, level_time_c(i,0), TXT_SML);
        menu_text(30,  -7 * h, -15 * h/4, c0, c1, level_time_n(i,0), TXT_SML);
        menu_text(31,  -2 * h, -15 * h/4, c0, c1, level_time_s(i,0), TXT_SML);

        menu_text(32, -11 * h, -21 * h/4, c0, c1, level_time_c(i,1), TXT_SML);
        menu_text(33,  -7 * h, -21 * h/4, c0, c1, level_time_n(i,1), TXT_SML);
        menu_text(34,  -2 * h, -21 * h/4, c0, c1, level_time_s(i,1), TXT_SML);

        menu_text(35, -11 * h, -27 * h/4, c0, c1, level_time_c(i,2), TXT_SML);
        menu_text(36,  -7 * h, -27 * h/4, c0, c1, level_time_n(i,2), TXT_SML);
        menu_text(37,  -2 * h, -27 * h/4, c0, c1, level_time_s(i,2), TXT_SML);


        menu_text(38,  +2 * h, -15 * h/4, c0, c1, level_coin_s(i,0), TXT_SML);
        menu_text(39,  +7 * h, -15 * h/4, c0, c1, level_coin_n(i,0), TXT_SML);
        menu_text(40, +11 * h, -15 * h/4, c0, c1, level_coin_c(i,0), TXT_SML);

        menu_text(41,  +2 * h, -21 * h/4, c0, c1, level_coin_s(i,1), TXT_SML);
        menu_text(42,  +7 * h, -21 * h/4, c0, c1, level_coin_n(i,1), TXT_SML);
        menu_text(43, +11 * h, -21 * h/4, c0, c1, level_coin_c(i,1), TXT_SML);

        menu_text(44,  +2 * h, -27 * h/4, c0, c1, level_coin_s(i,2), TXT_SML);
        menu_text(45,  +7 * h, -27 * h/4, c0, c1, level_coin_n(i,2), TXT_SML);
        menu_text(46, +11 * h, -27 * h/4, c0, c1, level_coin_c(i,2), TXT_SML);
    }
}

#define COL0(i) (level_opened(i) ? c_white : c_yellow)
#define COL1(i) (level_opened(i) ? c_white : c_red)
#define OPN(i)  (level_opened(i) ? i : -1)

static void start_enter(void)
{
    int i;
    int w;
    int h;

    shot_level = -1;

    level_init();

    text_size("0", TXT_SML, &w, &h);
    menu_init(47, 29, 0);

    /* Text elements */

    menu_text(0,  -8 * h, +33 * h / 4, c_white, c_white, "Back", TXT_SML);
    menu_text(1,  -9 * h, +27 * h / 4, COL0( 1), COL1( 1), "01", TXT_SML);
    menu_text(2,  -7 * h, +27 * h / 4, COL0( 2), COL1( 2), "02", TXT_SML);
    menu_text(3,  -5 * h, +27 * h / 4, COL0( 3), COL1( 3), "03", TXT_SML);
    menu_text(4,  -3 * h, +27 * h / 4, COL0( 4), COL1( 4), "04", TXT_SML);
    menu_text(5,  -1 * h, +27 * h / 4, COL0( 5), COL1( 5), "05", TXT_SML);
    menu_text(6,  -9 * h, +21 * h / 4, COL0( 6), COL1( 6), "06", TXT_SML);
    menu_text(7,  -7 * h, +21 * h / 4, COL0( 7), COL1( 7), "07", TXT_SML);
    menu_text(8,  -5 * h, +21 * h / 4, COL0( 8), COL1( 8), "08", TXT_SML);
    menu_text(9,  -3 * h, +21 * h / 4, COL0( 9), COL1( 9), "09", TXT_SML);
    menu_text(10, -1 * h, +21 * h / 4, COL0(10), COL1(10), "10", TXT_SML);
    menu_text(11, -9 * h, +15 * h / 4, COL0(11), COL1(11), "11", TXT_SML);
    menu_text(12, -7 * h, +15 * h / 4, COL0(12), COL1(12), "12", TXT_SML);
    menu_text(13, -5 * h, +15 * h / 4, COL0(13), COL1(13), "13", TXT_SML);
    menu_text(14, -3 * h, +15 * h / 4, COL0(14), COL1(14), "14", TXT_SML);
    menu_text(15, -1 * h, +15 * h / 4, COL0(15), COL1(15), "15", TXT_SML);
    menu_text(16, -9 * h,  +9 * h / 4, COL0(16), COL1(16), "16", TXT_SML);
    menu_text(17, -7 * h,  +9 * h / 4, COL0(17), COL1(17), "17", TXT_SML);
    menu_text(18, -5 * h,  +9 * h / 4, COL0(18), COL1(18), "18", TXT_SML);
    menu_text(19, -3 * h,  +9 * h / 4, COL0(19), COL1(19), "19", TXT_SML);
    menu_text(20, -1 * h,  +9 * h / 4, COL0(20), COL1(20), "20", TXT_SML);
    menu_text(21, -9 * h,  +3 * h / 4, COL0(21), COL1(21), "21", TXT_SML);
    menu_text(22, -7 * h,  +3 * h / 4, COL0(22), COL1(22), "22", TXT_SML);
    menu_text(23, -5 * h,  +3 * h / 4, COL0(23), COL1(23), "23", TXT_SML);
    menu_text(24, -3 * h,  +3 * h / 4, COL0(24), COL1(24), "24", TXT_SML);
    menu_text(25, -1 * h,  +3 * h / 4, COL0(25), COL1(25), "25", TXT_SML);

    menu_text(26, -6 * h,  -9 * h / 4, c_yellow, c_red, "Best Times", TXT_SML);
    menu_text(27, +6 * h,  -9 * h / 4, c_yellow, c_red, "Most Coins", TXT_SML);
    menu_text(28, +8 * h, +33 * h / 4, c_yellow, c_red, "Level",      TXT_SML);

    start_motion(0);

    /* Active items */

    menu_item(0,  -8 * h, +33 * h / 4, 4 * h, 3 * h / 2);
    menu_item(1,  -9 * h, +27 * h / 4, 2 * h, 3 * h / 2);
    menu_item(2,  -7 * h, +27 * h / 4, 2 * h, 3 * h / 2);
    menu_item(3,  -5 * h, +27 * h / 4, 2 * h, 3 * h / 2);
    menu_item(4,  -3 * h, +27 * h / 4, 2 * h, 3 * h / 2);
    menu_item(5,  -1 * h, +27 * h / 4, 2 * h, 3 * h / 2);
    menu_item(6,  -9 * h, +21 * h / 4, 2 * h, 3 * h / 2);
    menu_item(7,  -7 * h, +21 * h / 4, 2 * h, 3 * h / 2);
    menu_item(8,  -5 * h, +21 * h / 4, 2 * h, 3 * h / 2);
    menu_item(9,  -3 * h, +21 * h / 4, 2 * h, 3 * h / 2);
    menu_item(10, -1 * h, +21 * h / 4, 2 * h, 3 * h / 2);
    menu_item(11, -9 * h, +15 * h / 4, 2 * h, 3 * h / 2);
    menu_item(12, -7 * h, +15 * h / 4, 2 * h, 3 * h / 2);
    menu_item(13, -5 * h, +15 * h / 4, 2 * h, 3 * h / 2);
    menu_item(14, -3 * h, +15 * h / 4, 2 * h, 3 * h / 2);
    menu_item(15, -1 * h, +15 * h / 4, 2 * h, 3 * h / 2);
    menu_item(16, -9 * h,  +9 * h / 4, 2 * h, 3 * h / 2);
    menu_item(17, -7 * h,  +9 * h / 4, 2 * h, 3 * h / 2);
    menu_item(18, -5 * h,  +9 * h / 4, 2 * h, 3 * h / 2);
    menu_item(19, -3 * h,  +9 * h / 4, 2 * h, 3 * h / 2);
    menu_item(20, -1 * h,  +9 * h / 4, 2 * h, 3 * h / 2);
    menu_item(21, -9 * h,  +3 * h / 4, 2 * h, 3 * h / 2);
    menu_item(22, -7 * h,  +3 * h / 4, 2 * h, 3 * h / 2);
    menu_item(23, -5 * h,  +3 * h / 4, 2 * h, 3 * h / 2);
    menu_item(24, -3 * h,  +3 * h / 4, 2 * h, 3 * h / 2);
    menu_item(25, -1 * h,  +3 * h / 4, 2 * h, 3 * h / 2);

    menu_item(26, -6 * h, -18 * h / 4, 12 * h - 4, 12 * h / 2);
    menu_item(27, +6 * h, -18 * h / 4, 12 * h - 4, 12 * h / 2);
    menu_item(28, +8 * h, +33 * h / 4,  4 * h - 4,  3 * h / 2); 

    menu_stat(0, 0);

    for (i = 1; i < 26; i++)
        if (level_opened(i))
            menu_stat(i, 0);

    /* Item linkings for menu traversal */

    menu_link(0, -1,       OPN(1),  -1,      -1);
    menu_link(1,  0,       OPN(6),  -1,      OPN(2));
    menu_link(2,  0,       OPN(7),  OPN(1),  OPN(3));
    menu_link(3,  0,       OPN(8),  OPN(2),  OPN(4));
    menu_link(4,  0,       OPN(9),  OPN(3),  OPN(5));
    menu_link(5,  0,       OPN(10), OPN(4),  OPN(6));
    menu_link(6,  OPN(1),  OPN(11), OPN(5),  OPN(7));
    menu_link(7,  OPN(2),  OPN(12), OPN(6),  OPN(8));
    menu_link(8,  OPN(3),  OPN(13), OPN(7),  OPN(9));
    menu_link(9,  OPN(4),  OPN(14), OPN(8),  OPN(10));
    menu_link(10, OPN(5),  OPN(15), OPN(9),  OPN(11));
    menu_link(11, OPN(6),  OPN(16), OPN(10), OPN(12));
    menu_link(12, OPN(7),  OPN(17), OPN(11), OPN(13));
    menu_link(13, OPN(8),  OPN(18), OPN(12), OPN(14));
    menu_link(14, OPN(9),  OPN(19), OPN(13), OPN(15));
    menu_link(15, OPN(10), OPN(20), OPN(14), OPN(16));
    menu_link(16, OPN(11), OPN(21), OPN(15), OPN(17));
    menu_link(17, OPN(12), OPN(22), OPN(16), OPN(18));
    menu_link(18, OPN(13), OPN(23), OPN(17), OPN(19));
    menu_link(19, OPN(14), OPN(24), OPN(18), OPN(20));
    menu_link(20, OPN(15), OPN(25), OPN(19), OPN(21));
    menu_link(21, OPN(16), -1,      OPN(20), OPN(22));
    menu_link(22, OPN(17), -1,      OPN(21), OPN(23));
    menu_link(23, OPN(18), -1,      OPN(22), OPN(24));
    menu_link(24, OPN(19), -1,      OPN(23), OPN(25));
    menu_link(25, OPN(20), -1,      OPN(24), -1);

    /* Position the level shot. */

    shot_x0 = config_w() / 2;
    shot_y0 = config_h() / 2;
    shot_x1 = config_w() / 2 +  10 * h;
    shot_y1 = config_h() / 2 +  15 * h / 2;

    SDL_ShowCursor(SDL_ENABLE);
}

static void start_leave(void)
{
    SDL_ShowCursor(SDL_DISABLE);

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

    if ((i = menu_point(x, y)) >= 0)
        start_motion(i);

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
        start_motion(i);

    return 1;
}

static int start_buttn(int b, int d)
{
    if (config_button_a(b) && d == 1) return start_action(menu_buttn());
    if (config_button_b(b) && d == 1) return goto_state(&st_title);
    if (config_button_X(b) && d == 1) return goto_state(&st_title);
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
    if (config_button_a(b) && d == 1) return goto_state(&st_ready);
    if (config_button_X(b) && d == 1) return goto_state(&st_over);
    return 1;
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
    if (config_button_a(b) && d == 1) return goto_state(&st_play);
    if (config_button_X(b) && d == 1) return goto_state(&st_over);
    return 1;
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
    if (config_button_a(b) && d == 1) return goto_state(&st_play);
    if (config_button_X(b) && d == 1) return goto_state(&st_over);
    return 1;
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
    if (config_button_X(b) && d == 1)
        return goto_state(&st_over);

    return 1;
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
        if (level_goal())
            goto_state(&st_score);
        else
        {
            if (level_pass())
                goto_state(&st_level);
            else
                goto_state(&st_done);
        }
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
    if (config_button_a(b) && d == 1) return goal_click(0, 1);
    if (config_button_X(b) && d == 1) return goto_state(&st_over);
    return 1;
}

/*---------------------------------------------------------------------------*/

static int score_action(int i)
{
    int w, h, l = strlen(player);

    if (i == 28)
    {
        /* Ok selected  */

        if (level_pass())
            goto_state(&st_level);
        else
            goto_state(&st_done);

        return 1;
    }
    if (i == 27)
    {
        /* Backspace selected */

        if (l > 0)
            player[l - 1] = 0;
    }
    else
    {
        /* Letter selected */

        if (l < MAXNAM - 1)
        {
            player[l + 0] = i + 'A' - 1;
            player[l + 1] = 0;
        }
    }

    text_size("M", TXT_MED, &w, &h);
    menu_text(0, 0, h & (~3), c_yellow, c_white, player, TXT_MED);

    return 1;
}

static void score_enter(void)
{
    int i;
    int sw, mw, lw;
    int sh, mh, lh;

    text_size("M", TXT_SML, &sw, &sh);
    text_size("M", TXT_MED, &mw, &mh);
    text_size("M", TXT_LRG, &lw, &lh);
    sw *= 2;
    sh *= 2;

    menu_init(30, 30, 28);

    /* Text elements */

    menu_text(0,  0,     mh, c_yellow, c_white, player,        TXT_MED);
    menu_text(29, 0, 3 * mh, c_yellow, c_red,   "New Record!", TXT_LRG);

    menu_text(1,  -3 * sw, -1 * sh, c_white, c_white, "A",  TXT_SML);
    menu_text(2,  -2 * sw, -1 * sh, c_white, c_white, "B",  TXT_SML);
    menu_text(3,  -1 * sw, -1 * sh, c_white, c_white, "C",  TXT_SML);
    menu_text(4,   0 * sw, -1 * sh, c_white, c_white, "D",  TXT_SML);
    menu_text(5,  +1 * sw, -1 * sh, c_white, c_white, "E",  TXT_SML);
    menu_text(6,  +2 * sw, -1 * sh, c_white, c_white, "F",  TXT_SML);
    menu_text(7,  +3 * sw, -1 * sh, c_white, c_white, "G",  TXT_SML);
    menu_text(8,  -3 * sw, -2 * sh, c_white, c_white, "H",  TXT_SML);
    menu_text(9,  -2 * sw, -2 * sh, c_white, c_white, "I",  TXT_SML);
    menu_text(10, -1 * sw, -2 * sh, c_white, c_white, "J",  TXT_SML);
    menu_text(11,  0 * sw, -2 * sh, c_white, c_white, "K",  TXT_SML);
    menu_text(12, +1 * sw, -2 * sh, c_white, c_white, "L",  TXT_SML);
    menu_text(13, +2 * sw, -2 * sh, c_white, c_white, "M",  TXT_SML);
    menu_text(14, +3 * sw, -2 * sh, c_white, c_white, "N",  TXT_SML);
    menu_text(15, -3 * sw, -3 * sh, c_white, c_white, "O",  TXT_SML);
    menu_text(16, -2 * sw, -3 * sh, c_white, c_white, "P",  TXT_SML);
    menu_text(17, -1 * sw, -3 * sh, c_white, c_white, "Q",  TXT_SML);
    menu_text(18,  0 * sw, -3 * sh, c_white, c_white, "R",  TXT_SML);
    menu_text(19, +1 * sw, -3 * sh, c_white, c_white, "S",  TXT_SML);
    menu_text(20, +2 * sw, -3 * sh, c_white, c_white, "T",  TXT_SML);
    menu_text(21, +3 * sw, -3 * sh, c_white, c_white, "U",  TXT_SML);
    menu_text(22, -3 * sw, -4 * sh, c_white, c_white, "V",  TXT_SML);
    menu_text(23, -2 * sw, -4 * sh, c_white, c_white, "W",  TXT_SML);
    menu_text(24, -1 * sw, -4 * sh, c_white, c_white, "X",  TXT_SML);
    menu_text(25,  0 * sw, -4 * sh, c_white, c_white, "Y",  TXT_SML);
    menu_text(26, +1 * sw, -4 * sh, c_white, c_white, "Z",  TXT_SML);
    menu_text(27, +2 * sw, -4 * sh, c_green, c_white, "<",  TXT_SML);
    menu_text(28, +3 * sw, -4 * sh, c_green, c_white, "Ok", TXT_SML);

    /* Active elements. */

    menu_item(0,  0,     mh,  mw * MAXNAM,     mh);
    menu_item(29, 0, 3 * mh, 0.9 * config_w(), lh);

    menu_item(1,  -3 * sw, -1 * sh, sw, sh);
    menu_item(2,  -2 * sw, -1 * sh, sw, sh);
    menu_item(3,  -1 * sw, -1 * sh, sw, sh);
    menu_item(4,   0 * sw, -1 * sh, sw, sh);
    menu_item(5,  +1 * sw, -1 * sh, sw, sh);
    menu_item(6,  +2 * sw, -1 * sh, sw, sh);
    menu_item(7,  +3 * sw, -1 * sh, sw, sh);
    menu_item(8,  -3 * sw, -2 * sh, sw, sh);
    menu_item(9,  -2 * sw, -2 * sh, sw, sh);
    menu_item(10, -1 * sw, -2 * sh, sw, sh);
    menu_item(11,  0 * sw, -2 * sh, sw, sh);
    menu_item(12, +1 * sw, -2 * sh, sw, sh);
    menu_item(13, +2 * sw, -2 * sh, sw, sh);
    menu_item(14, +3 * sw, -2 * sh, sw, sh);
    menu_item(15, -3 * sw, -3 * sh, sw, sh);
    menu_item(16, -2 * sw, -3 * sh, sw, sh);
    menu_item(17, -1 * sw, -3 * sh, sw, sh);
    menu_item(18,  0 * sw, -3 * sh, sw, sh);
    menu_item(19, +1 * sw, -3 * sh, sw, sh);
    menu_item(20, +2 * sw, -3 * sh, sw, sh);
    menu_item(21, +3 * sw, -3 * sh, sw, sh);
    menu_item(22, -3 * sw, -4 * sh, sw, sh);
    menu_item(23, -2 * sw, -4 * sh, sw, sh);
    menu_item(24, -1 * sw, -4 * sh, sw, sh);
    menu_item(25,  0 * sw, -4 * sh, sw, sh);
    menu_item(26, +1 * sw, -4 * sh, sw, sh);
    menu_item(27, +2 * sw, -4 * sh, sw, sh);
    menu_item(28, +3 * sw, -4 * sh, sw, sh);

    for (i = 1; i < 29; i++)
        menu_stat(i, 0);

    /* Menu traversal linkages. */

    menu_link(1,  -1,  8, -1,  2);
    menu_link(2,  -1,  9,  1,  3);
    menu_link(3,  -1, 10,  2,  4);
    menu_link(4,  -1, 11,  3,  5);
    menu_link(5,  -1, 12,  4,  6);
    menu_link(6,  -1, 13,  5,  7);
    menu_link(7,  -1, 14,  6,  8);
    menu_link(8,   1, 15,  7,  9);
    menu_link(9,   2, 16,  8, 10);
    menu_link(10,  3, 17,  9, 11);
    menu_link(11,  4, 18, 10, 12);
    menu_link(12,  5, 19, 11, 13);
    menu_link(13,  6, 20, 12, 14);
    menu_link(14,  7, 21, 13, 15);
    menu_link(15,  8, 22, 14, 16);
    menu_link(16,  9, 23, 15, 17);
    menu_link(17, 10, 24, 16, 18);
    menu_link(18, 11, 25, 17, 19);
    menu_link(19, 12, 26, 18, 20);
    menu_link(20, 13, 27, 19, 21);
    menu_link(21, 14, 28, 20, 22);
    menu_link(22, 15, -1, 21, 23);
    menu_link(23, 16, -1, 22, 24);
    menu_link(24, 17, -1, 23, 25);
    menu_link(25, 18, -1, 24, 26);
    menu_link(26, 19, -1, 25, 27);
    menu_link(27, 20, -1, 26, 28);
    menu_link(28, 20, -1, 27, -1);

    audio_play(AUD_OVER, 1.f);

    SDL_ShowCursor(SDL_ENABLE);
}

static void score_leave(void)
{
    SDL_ShowCursor(SDL_DISABLE);

    menu_free();
}

static void score_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0);
    menu_paint();
}

static int score_point(int x, int y, int dx, int dy)
{
    menu_point(x, y);
    return 1;
}

static int score_click(int b, int d)
{
    return (b < 0 && d == 1) ? score_action(menu_click()) : 1;
}

static int score_keybd(int c)
{
    if (c == SDLK_ESCAPE)
        goto_state(&st_over);
    return 1;
}

static int score_stick(int a, int v)
{
    menu_stick(a, v);
    return 1;
}

static int score_buttn(int b, int d)
{
    if (config_button_a(b) && d == 1) return score_action(menu_buttn());
    if (config_button_X(b) && d == 1) return goto_state(&st_over);
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
    if (config_button_a(b) && d == 1) return fall_click(0, 1);
    if (config_button_X(b) && d == 1) return goto_state(&st_over);
    return 1;
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

    audio_play(AUD_TIME,  1.f);
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
    if (config_button_a(b) && d == 1) return time_click(0, 1);
    if (config_button_X(b) && d == 1) return goto_state(&st_over);
    return 1;
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

struct state st_score = {
    score_enter,
    score_leave,
    score_paint,
    NULL,
    score_point,
    score_click,
    score_keybd,
    score_stick,
    score_buttn,
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

