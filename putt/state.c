/*   
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERPUTT is  free software; you can redistribute  it and/or modify
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
#include "vec3.h"
#include "geom.h"
#include "game.h"
#include "menu.h"
#include "text.h"
#include "solid.h"
#include "state.h"
#include "back.h"
#include "hole.h"
#include "image.h"
#include "audio.h"
#include "config.h"

/*---------------------------------------------------------------------------*/

static float state_time   = 0.f;
static struct state *state = &st_null;

float time_state(void)
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (state && state->paint) state->paint();
}

int st_timer(float t)
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

/*---------------------------------------------------------------------------*/

static void score_card(const char *title,
                       const GLfloat *t0,
                       const GLfloat *t1)
{
    const GLfloat *c0 = c_yellow;
    const GLfloat *c1 = c_white;
    const GLfloat *c2 = c_red;
    const GLfloat *c3 = c_white;

    int w, h, i, k = 0;

    text_size("0", TXT_SML, &w, &h);
    w = 3 * h / 2;

    menu_item(0,  0,     +(14 * h) / 2, 12 * w - 4, 2 * h);
    menu_item(1,  0,     +(5  * h) / 2, 12 * w - 4, 6 * h + 8);
    menu_item(2,  0,     -(9  * h) / 2, 12 * w - 4, 6 * h + 8);
    menu_item(3, +7 * w, -(9  * h) / 2,  2 * w - 4, 6 * h + 8);

    menu_text(k++, 0, +7*h, t0, t1, title, TXT_MED);

    menu_text(k++, -4*w, +5*h, c0, c2,  "01",  TXT_SML);
    menu_text(k++, -3*w, +5*h, c0, c2,  "02",  TXT_SML);
    menu_text(k++, -2*w, +5*h, c0, c2,  "03",  TXT_SML);
    menu_text(k++, -1*w, +5*h, c0, c2,  "04",  TXT_SML);
    menu_text(k++,  0*w, +5*h, c0, c2,  "05",  TXT_SML);
    menu_text(k++, +1*w, +5*h, c0, c2,  "06",  TXT_SML);
    menu_text(k++, +2*w, +5*h, c0, c2,  "07",  TXT_SML);
    menu_text(k++, +3*w, +5*h, c0, c2,  "08",  TXT_SML);
    menu_text(k++, +4*w, +5*h, c0, c2,  "09",  TXT_SML);
    menu_text(k++, +5*w, +5*h, c0, c2, "Out",  TXT_SML);

    for (i = 0; i <= 4; i++)
    {
        c3 = c_play[i];

        menu_text(k++, -5*w, (+4-i)*h, c3, c1, hole_player(i),    TXT_SML);
        menu_text(k++, -4*w, (+4-i)*h, c3, c1, hole_score(1, i),  TXT_SML);
        menu_text(k++, -3*w, (+4-i)*h, c3, c1, hole_score(2, i),  TXT_SML);
        menu_text(k++, -2*w, (+4-i)*h, c3, c1, hole_score(3, i),  TXT_SML);
        menu_text(k++, -1*w, (+4-i)*h, c3, c1, hole_score(4, i),  TXT_SML);
        menu_text(k++,  0*w, (+4-i)*h, c3, c1, hole_score(5, i),  TXT_SML);
        menu_text(k++, +1*w, (+4-i)*h, c3, c1, hole_score(6, i),  TXT_SML);
        menu_text(k++, +2*w, (+4-i)*h, c3, c1, hole_score(7, i),  TXT_SML);
        menu_text(k++, +3*w, (+4-i)*h, c3, c1, hole_score(8, i),  TXT_SML);
        menu_text(k++, +4*w, (+4-i)*h, c3, c1, hole_score(9, i),  TXT_SML);
        menu_text(k++, +5*w, (+4-i)*h, c3, c1, hole_out(i),       TXT_SML);
    }

    menu_text(k++, -4*w, -2*h, c0, c2,  "10", TXT_SML);
    menu_text(k++, -3*w, -2*h, c0, c2,  "11", TXT_SML);
    menu_text(k++, -2*w, -2*h, c0, c2,  "12", TXT_SML);
    menu_text(k++, -1*w, -2*h, c0, c2,  "13", TXT_SML);
    menu_text(k++,  0*w, -2*h, c0, c2,  "14", TXT_SML);
    menu_text(k++, +1*w, -2*h, c0, c2,  "15", TXT_SML);
    menu_text(k++, +2*w, -2*h, c0, c2,  "16", TXT_SML);
    menu_text(k++, +3*w, -2*h, c0, c2,  "17", TXT_SML);
    menu_text(k++, +4*w, -2*h, c0, c2,  "18", TXT_SML);
    menu_text(k++, +5*w, -2*h, c0, c2,  "In", TXT_SML);
    menu_text(k++, +7*w, -2*h, c0, c2, "Tot", TXT_SML);

    for (i = 0; i <= 4; i++)
    {
        c3 = c_play[i];

        menu_text(k++, -5*w, (-3-i)*h, c3, c1, hole_player(i),     TXT_SML);
        menu_text(k++, -4*w, (-3-i)*h, c3, c1, hole_score(10, i),  TXT_SML);
        menu_text(k++, -3*w, (-3-i)*h, c3, c1, hole_score(11, i),  TXT_SML);
        menu_text(k++, -2*w, (-3-i)*h, c3, c1, hole_score(12, i),  TXT_SML);
        menu_text(k++, -1*w, (-3-i)*h, c3, c1, hole_score(13, i),  TXT_SML);
        menu_text(k++,  0*w, (-3-i)*h, c3, c1, hole_score(14, i),  TXT_SML);
        menu_text(k++, +1*w, (-3-i)*h, c3, c1, hole_score(15, i),  TXT_SML);
        menu_text(k++, +2*w, (-3-i)*h, c3, c1, hole_score(16, i),  TXT_SML);
        menu_text(k++, +3*w, (-3-i)*h, c3, c1, hole_score(17, i),  TXT_SML);
        menu_text(k++, +4*w, (-3-i)*h, c3, c1, hole_score(18, i),  TXT_SML);
        menu_text(k++, +5*w, (-3-i)*h, c3, c1, hole_in(i),         TXT_SML);
        menu_text(k++, +7*w, (-3-i)*h, c3, c1, hole_tot(i),        TXT_SML);
    }

}

/*---------------------------------------------------------------------------*/

static void null_enter(void)
{
    hud_free();
    swch_free();
    jump_free();
    flag_free();
    mark_free();
    ball_free();
    text_free();
}

static void null_leave(void)
{
    glext_init();
    text_init(config_get(CONFIG_HEIGHT));
    ball_init(config_get(CONFIG_GEOMETRY));
    mark_init(config_get(CONFIG_GEOMETRY));
    flag_init(config_get(CONFIG_GEOMETRY));
    jump_init(config_get(CONFIG_GEOMETRY));
    swch_init(config_get(CONFIG_GEOMETRY));
    hud_init();
}

/*---------------------------------------------------------------------------*/

#define TITLE_PLAY 1
#define TITLE_CONF 2
#define TITLE_EXIT 3

#define STR_TITLE " Neverputt "
#define STR_PLAY  " Play "
#define STR_CONF  " Options "
#define STR_EXIT  " Exit "

static int title_action(int i)
{
    switch (i)
    {
    case TITLE_PLAY: audio_play(AUD_MENU, 1.f); goto_state(&st_party); break;
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

    menu_stat(0,          -1);
    menu_stat(TITLE_PLAY, +1);
    menu_stat(TITLE_CONF,  0);
    menu_stat(TITLE_EXIT,  0);

    hole_init();
    hole_goto(rand() % curr_count(), 1);
}

static void title_leave(void)
{
    hole_free();
    menu_free();
}

static void title_paint(void)
{
    game_draw(0);
    menu_paint();
}

static int title_timer(float dt)
{
    float g[3] = { 0.f, 0.f, 0.f };

    game_step(g, dt);
    game_set_fly(fcosf(time_state() / 10.f));

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
#define CONF_SNDDN 13
#define CONF_SNDUP 14
#define CONF_MUSDN 15
#define CONF_MUSUP 16
#define CONF_BACK  17

static int conf_action(int i)
{
    int f = config_get(CONFIG_FULLSCREEN);
    int w = config_get(CONFIG_WIDTH);
    int h = config_get(CONFIG_HEIGHT);
    int s = config_get(CONFIG_SOUND_VOLUME);
    int m = config_get(CONFIG_MUSIC_VOLUME);
    int r = 1;
    int w2;
    char str[8];

    text_size("MMMMMMMMM", TXT_SML, &w, &h);
    w2 = w / 2;

    switch (i)
    {
    case CONF_FULL:
        goto_state(&st_null);
        r = config_mode(1, w, h);
        goto_state(&st_conf);
        break;

    case CONF_WIN:
        goto_state(&st_null);
        r = config_mode(0, w, h);
        goto_state(&st_conf);
        break;

    case CONF_16x12:
        goto_state(&st_null);
        r = config_mode(f, 1600, 1200);
        goto_state(&st_conf);
        break;

    case CONF_12x10:
        goto_state(&st_null);
        r = config_mode(f, 1280, 1024);
        goto_state(&st_conf);
        break;

    case CONF_10x7:
        goto_state(&st_null);
        r = config_mode(f, 1024, 768);
        goto_state(&st_conf);
        break;
            
    case CONF_8x6:
        goto_state(&st_null);
        r = config_mode(f, 800, 600);
        goto_state(&st_conf);
        break;

    case CONF_6x4:
        goto_state(&st_null);
        r = config_mode(f, 640, 480);
        goto_state(&st_conf);
        break;

    case CONF_TEXHI:
        goto_state(&st_null);
        config_set(CONFIG_TEXTURES, 1);
        goto_state(&st_conf);
        break;

    case CONF_TEXLO:
        goto_state(&st_null);
        config_set(CONFIG_TEXTURES, 2);
        goto_state(&st_conf);
        break;

    case CONF_GEOHI:
        goto_state(&st_null);
        config_set(CONFIG_GEOMETRY, 1);
        goto_state(&st_conf);
        break;

    case CONF_GEOLO:
        goto_state(&st_null);
        config_set(CONFIG_GEOMETRY, 0);
        goto_state(&st_conf);
        break;

    case CONF_AUDHI:
        audio_free();
        config_set(CONFIG_AUDIO_RATE, 44100);
        config_set(CONFIG_AUDIO_BUFF, AUDIO_BUFF_HI);
        audio_init();
        goto_state(&st_conf);
        break;

    case CONF_AUDLO:
        audio_free();
        config_set(CONFIG_AUDIO_RATE, 22050);
        config_set(CONFIG_AUDIO_BUFF, AUDIO_BUFF_LO);
        audio_init();
        goto_state(&st_conf);
        break;

    case CONF_SNDDN:
        config_set(CONFIG_SOUND_VOLUME, s - 1);
        sprintf(str, "%02d", s - 1);
        audio_volume(s - 1, m);
        menu_text(33, +w2, -3 * h, c_yellow, c_red, str, TXT_SML);
        audio_play(AUD_BUMP, 1.f);
        break;
        
    case CONF_SNDUP:
        config_set(CONFIG_SOUND_VOLUME, s + 1);
        sprintf(str, "%02d", s + 1);
        audio_volume(s + 1, m);
        menu_text(33, +w2, -3 * h, c_yellow, c_red, str, TXT_SML);
        audio_play(AUD_BUMP, 1.f);
        break;

    case CONF_MUSDN:
        config_set(CONFIG_MUSIC_VOLUME, m - 1);
        sprintf(str, "%02d", m - 1);
        audio_volume(s, m - 1);
        menu_text(34, +w2, -4 * h, c_yellow, c_red, str, TXT_SML);
        break;
        
    case CONF_MUSUP:
        config_set(CONFIG_MUSIC_VOLUME, m + 1);
        sprintf(str, "%02d", m + 1);
        audio_volume(s, m + 1);
        menu_text(34, +w2, -4 * h, c_yellow, c_red, str, TXT_SML);
        break;

    case CONF_BACK:
        goto_state(&st_title);
        break;
    }
    return r;
}

static void conf_enter(void)
{
    static int value = CONF_FULL;
    char snds[16];
    char muss[16];

    int w, w2, w4;
    int h, h2;
    
    /* Compute menu sizes from font sizes */

    text_size("MMMMMMMMM", TXT_SML, &w, &h);
    w2 = w / 2;
    w4 = w / 4;
    h2 = h / 2;

    sprintf(snds, "%02d", config_get(CONFIG_SOUND_VOLUME));
    sprintf(muss, "%02d", config_get(CONFIG_MUSIC_VOLUME));

    menu_init(36, 25, value);

    /* Text elements */

    menu_text(0,  -w2, +h2 + 4 * h, c_yellow, c_red,   "Options",    TXT_MED);
    menu_text(1,  -w2,      +3 * h, c_white,  c_white, "Fullscreen", TXT_SML);
    menu_text(2,  -w2,      +2 * h, c_white,  c_white, "Window",     TXT_SML);
    menu_text(3,  +w2 - w4, +5 * h, c_white,  c_white, "1600",       TXT_SML);
    menu_text(4,  +w2 - w4, +4 * h, c_white,  c_white, "1280",       TXT_SML);
    menu_text(5,  +w2 - w4, +3 * h, c_white,  c_white, "1024",       TXT_SML);
    menu_text(6,  +w2 - w4, +2 * h, c_white,  c_white,  "800",       TXT_SML);
    menu_text(7,  +w2 - w4,  1 * h, c_white,  c_white,  "640",       TXT_SML);
    menu_text(8,  +w2,      +5 * h, c_white,  c_white,    "x",       TXT_SML);
    menu_text(9,  +w2,      +4 * h, c_white,  c_white,    "x",       TXT_SML);
    menu_text(10, +w2,      +3 * h, c_white,  c_white,    "x",       TXT_SML);
    menu_text(11, +w2,      +2 * h, c_white,  c_white,    "x",       TXT_SML);
    menu_text(12, +w2,       1 * h, c_white,  c_white,    "x",       TXT_SML);
    menu_text(13, +w2 + w4, +5 * h, c_white,  c_white, "1200",       TXT_SML);
    menu_text(14, +w2 + w4, +4 * h, c_white,  c_white, "1024",       TXT_SML);
    menu_text(15, +w2 + w4, +3 * h, c_white,  c_white,  "768",       TXT_SML);
    menu_text(16, +w2 + w4, +2 * h, c_white,  c_white,  "600",       TXT_SML);
    menu_text(17, +w2 + w4,  1 * h, c_white,  c_white,  "480",       TXT_SML);
    menu_text(18, -w2,       0 * h, c_yellow, c_red,   "Textures",   TXT_SML);
    menu_text(19, -w2,      -1 * h, c_yellow, c_red,   "Geometry",   TXT_SML);
    menu_text(20, -w2,      -2 * h, c_yellow, c_red,   "Audio",      TXT_SML);
    menu_text(21, -w2,      -3 * h, c_yellow, c_red,   "Sound Vol",  TXT_SML);
    menu_text(22, -w2,      -4 * h, c_yellow, c_red,   "Music Vol",  TXT_SML);
    menu_text(23, +w2 - w4,  0 * h, c_white,  c_white, "High",       TXT_SML);
    menu_text(24, +w2 - w4, -1 * h, c_white,  c_white, "High",       TXT_SML);
    menu_text(25, +w2 - w4, -2 * h, c_white,  c_white, "High",       TXT_SML);
    menu_text(26, +w2 - w4, -3 * h, c_white,  c_white, "<",          TXT_SML);
    menu_text(27, +w2 - w4, -4 * h, c_white,  c_white, "<",          TXT_SML);
    menu_text(28, +w2 + w4,  0 * h, c_white,  c_white, "Low",        TXT_SML);
    menu_text(29, +w2 + w4, -1 * h, c_white,  c_white, "Low",        TXT_SML);
    menu_text(30, +w2 + w4, -2 * h, c_white,  c_white, "Low",        TXT_SML);
    menu_text(31, +w2 + w4, -3 * h, c_white,  c_white, ">",          TXT_SML);
    menu_text(32, +w2 + w4, -4 * h, c_white,  c_white, ">",          TXT_SML);
    menu_text(33, +w2,      -3 * h, c_yellow, c_red,   snds,         TXT_SML);
    menu_text(34, +w2,      -4 * h, c_yellow, c_red,   muss,         TXT_SML);
    menu_text(35, +w2,      -5 * h, c_white,  c_white, "Back",       TXT_SML);

    /* Active items */

    menu_item(CONF_FULL,  -w2,      +3 * h, w,  h);
    menu_item(CONF_WIN,   -w2,      +2 * h, w,  h);
    menu_item(CONF_16x12, +w2,      +5 * h, w,  h);
    menu_item(CONF_12x10, +w2,      +4 * h, w,  h);
    menu_item(CONF_10x7,  +w2,      +3 * h, w,  h);
    menu_item(CONF_8x6,   +w2,      +2 * h, w,  h);
    menu_item(CONF_6x4,   +w2,      +1 * h, w,  h);
    menu_item(CONF_TEXHI, +w2 - w4,  0 * h, w2, h);
    menu_item(CONF_TEXLO, +w2 + w4,  0 * h, w2, h);
    menu_item(CONF_GEOHI, +w2 - w4, -1 * h, w2, h);
    menu_item(CONF_GEOLO, +w2 + w4, -1 * h, w2, h);
    menu_item(CONF_AUDHI, +w2 - w4, -2 * h, w2, h);
    menu_item(CONF_AUDLO, +w2 + w4, -2 * h, w2, h);
    menu_item(CONF_SNDDN, +w2 - w4, -3 * h, w2, h);
    menu_item(CONF_SNDUP, +w2 + w4, -3 * h, w2, h);
    menu_item(CONF_MUSDN, +w2 - w4, -4 * h, w2, h);
    menu_item(CONF_MUSUP, +w2 + w4, -4 * h, w2, h);
    menu_item(CONF_BACK,  +w2,      -5 * h, w,  h);

    /* Inactive label padding */

    menu_item(18, -w2, +5 * h, w,  h);
    menu_item(19, -w2, +4 * h, w,  h);
    menu_item(20, -w2,  0 * h, w,  h);
    menu_item(21, -w2, -1 * h, w,  h);
    menu_item(22, -w2, -2 * h, w,  h);
    menu_item(23, -w2, -3 * h, w,  h);
    menu_item(24, -w2, -4 * h, w,  h);

    /* Item state */

    menu_stat(CONF_FULL,  (config_get(CONFIG_FULLSCREEN)) ? 1 : 0);
    menu_stat(CONF_WIN,   (config_get(CONFIG_FULLSCREEN)) ? 0 : 1);
    menu_stat(CONF_16x12, (config_get(CONFIG_WIDTH) == 1600)             ? 1 : 0);
    menu_stat(CONF_12x10, (config_get(CONFIG_WIDTH) == 1280)             ? 1 : 0);
    menu_stat(CONF_10x7,  (config_get(CONFIG_WIDTH) == 1024)             ? 1 : 0);
    menu_stat(CONF_8x6,   (config_get(CONFIG_WIDTH) ==  800)             ? 1 : 0);
    menu_stat(CONF_6x4,   (config_get(CONFIG_WIDTH) ==  640)             ? 1 : 0);
    menu_stat(CONF_TEXHI, (config_get(CONFIG_TEXTURES) == 1)             ? 1 : 0);
    menu_stat(CONF_TEXLO, (config_get(CONFIG_TEXTURES) == 2)             ? 1 : 0);
    menu_stat(CONF_GEOHI, (config_get(CONFIG_GEOMETRY) == 1)             ? 1 : 0);
    menu_stat(CONF_GEOLO, (config_get(CONFIG_GEOMETRY) == 0)             ? 1 : 0);
    menu_stat(CONF_AUDHI, (config_get(CONFIG_AUDIO_RATE) == 44100)         ? 1 : 0);
    menu_stat(CONF_AUDLO, (config_get(CONFIG_AUDIO_RATE) == 22050)         ? 1 : 0);
    menu_stat(CONF_SNDDN, 0);
    menu_stat(CONF_SNDUP, 0);
    menu_stat(CONF_MUSDN, 0);
    menu_stat(CONF_MUSUP, 0);
    menu_stat(CONF_BACK,  0);

    /* Disable unsupported modes. */

    if (!SDL_VideoModeOK(1600, 1200, 16, SDL_HWSURFACE))
        menu_stat(CONF_16x12, -1);
    if (!SDL_VideoModeOK(1280, 1024, 16, SDL_HWSURFACE))
        menu_stat(CONF_12x10, -1);
    if (!SDL_VideoModeOK(1024, 768,  16, SDL_HWSURFACE))
        menu_stat(CONF_10x7,  -1);
    if (!SDL_VideoModeOK(800,  600,  16, SDL_HWSURFACE))
        menu_stat(CONF_8x6,   -1);
    if (!SDL_VideoModeOK(640,  480,  16, SDL_HWSURFACE))
        menu_stat(CONF_8x6,   -1);

    value = -1;
}

static void conf_leave(void)
{
    config_save();
    menu_free();
}

static void conf_paint(void)
{
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

/*---------------------------------------------------------------------------*/

#define PARTY_T 0
#define PARTY_1 1
#define PARTY_2 2
#define PARTY_3 3
#define PARTY_4 4
#define PARTY_B 5

static int party_action(int i)
{
    switch (i)
    {
    case PARTY_1:
        audio_play(AUD_MENU, 1.f);
        hole_init();
        hole_goto(1, 1);
        goto_state(&st_next);
        break;
    case PARTY_2:
        audio_play(AUD_MENU, 1.f);
        hole_init();
        hole_goto(1, 2);
        goto_state(&st_next);
        break;
    case PARTY_3:
        audio_play(AUD_MENU, 1.f);
        hole_init();
        hole_goto(1, 3);
        goto_state(&st_next);
        break;
    case PARTY_4:
        audio_play(AUD_MENU, 1.f);
        hole_init();
        hole_goto(1, 4);
        goto_state(&st_next);
        break;
    case PARTY_B:
        audio_play(AUD_MENU, 1.f);
        goto_state(&st_title);
        break;
    }
    return 1;
}

static void party_enter(void)
{
    int mw, mh;
    int lw, lh, w = config_get(CONFIG_WIDTH) / 2;

    text_size("0", TXT_MED, &mw, &mh);
    text_size("0", TXT_LRG, &lw, &lh);

    menu_init(6, 6, 7);

    menu_text(PARTY_T, 0, +1 * lh, c_white,  c_white, "Players?",  TXT_MED);
    menu_text(PARTY_1, -3 * lw, 0, c_red,    c_white, "1", TXT_LRG);
    menu_text(PARTY_2, -1 * lw, 0, c_green,  c_white, "2", TXT_LRG);
    menu_text(PARTY_3, +1 * lw, 0, c_blue,   c_white, "3", TXT_LRG);
    menu_text(PARTY_4, +3 * lw, 0, c_yellow, c_white, "4", TXT_LRG);
    menu_text(PARTY_B, 0, -1 * lh, c_yellow, c_red,   "Back",      TXT_MED);

    menu_item(PARTY_T, 0, +1 * lh,  w, mh);
    menu_item(PARTY_1, -3 * lw, 0, 2 * lw, lh);
    menu_item(PARTY_2, -1 * lw, 0, 2 * lw, lh);
    menu_item(PARTY_3, +1 * lw, 0, 2 * lw, lh);
    menu_item(PARTY_4, +3 * lw, 0, 2 * lw, lh);
    menu_item(PARTY_B, 0, -1 * lh,  w, mh);

    menu_stat(PARTY_T, -1);
    menu_stat(PARTY_1,  0);
    menu_stat(PARTY_2,  0);
    menu_stat(PARTY_3,  0);
    menu_stat(PARTY_4,  0);
    menu_stat(PARTY_B,  0);

    audio_music_fade(2.0f);
}

static void party_leave(void)
{
    menu_free();
}

static void party_paint(void)
{
    menu_paint();
}

static int party_timer(float dt)
{
    return 1;
}

static int party_point(int x, int y, int dx, int dy)
{
    menu_point(x, y);
    return 1;
}

static int party_click(int b, int d)
{
    return (b < 0 && d == 1) ? party_action(menu_click()) : 1;
}

static int party_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

/*---------------------------------------------------------------------------*/

static void next_enter(void)
{
    const GLfloat *c0 = c_yellow;
    const GLfloat *c1 = c_red;
    const GLfloat *c2 = c_white;
    const GLfloat *c3 = c_play[curr_player()];

    char str1[MAXSTR];
    char str2[MAXSTR];

    int mw, mh;
    int sw, sh;
    int lw, lh;
    int  w,  h;

    text_size("0", TXT_SML, &sw, &sh);
    text_size("0", TXT_MED, &mw, &mh);
    text_size("0", TXT_LRG, &lw, &lh);

    w = config_get(CONFIG_WIDTH) / 2;
    h = -(lh + sh) / 2;

    sprintf(str1, "Hole %02d", curr_hole());
    sprintf(str2, "%d", curr_player());

    menu_init(3, 3, 4);

    menu_text(0, 0, +mh, c0, c1, str1,     TXT_MED);
    menu_text(1, 0,   0, c3, c2, "Player", TXT_SML);
    menu_text(2, 0,   h, c3, c2, str2,     TXT_LRG);

    menu_item(0, 0, +mh, w,      mh);
    menu_item(1, 0,   0, lw * 2, sh);
    menu_item(2, 0,   h, lw * 2, lh);

    if (curr_party() > 1)
        switch (curr_player())
        {
        case 1: audio_play(AUD_PLAYER1, 1.0f); break;
        case 2: audio_play(AUD_PLAYER2, 1.0f); break;
        case 3: audio_play(AUD_PLAYER3, 1.0f); break;
        case 4: audio_play(AUD_PLAYER4, 1.0f); break;
        }

    game_set_fly(1.f);
}

static void next_leave(void)
{
    menu_free();
}

static void next_paint(void)
{
    game_draw(0);
    menu_paint();
}

static int next_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_flyby) : 1;
}

static int next_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_over) : 1;
}

/*---------------------------------------------------------------------------*/

static void flyby_enter(void)
{
}

static void flyby_leave(void)
{
}

static void flyby_paint(void)
{
    game_draw(0);
    hud_draw();
}

static int flyby_timer(float dt)
{
    float t = time_state();

    if (dt > 0.f && t > 1.f)
        return goto_state(&st_stroke);
    else
        game_set_fly(1.f - t);

    return 1;
}

static int flyby_click(int b, int d)
{
    if (b < 0 && d == 1)
    {
        game_set_fly(0.f);
        return goto_state(&st_stroke);
    }
    return 1;
}

static int flyby_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_over) : 1;
}

/*---------------------------------------------------------------------------*/

static void stroke_enter(void)
{
    game_clr_mag();
    config_set(CONFIG_CAMERA, 2);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_WM_GrabInput(SDL_GRAB_ON);
}

static void stroke_leave(void)
{
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_ShowCursor(SDL_ENABLE);
    config_set(CONFIG_CAMERA, 0);
}

static void stroke_paint(void)
{
    game_draw(0);
    hud_draw();
}

static int stroke_point(int x, int y, int dx, int dy)
{
    game_set_rot(dx);
    game_set_mag(dy);
    return 1;
}

static int stroke_timer(float dt)
{
    float g[3] = { 0.f, 0.f, 0.f };
    
    game_update_view(dt);
    game_step(g, dt);
    return 1;
}

static int stroke_click(int b, int d)
{
    if (b < 0 && d == 1)
        return goto_state(&st_roll);

    return 1;
}

static int stroke_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_over) : 1;
}

/*---------------------------------------------------------------------------*/

static void roll_enter(void)
{
    game_putt();
}

static void roll_leave(void)
{
}

static void roll_paint(void)
{
    game_draw(0);
    hud_draw();
}

static int roll_timer(float dt)
{
    float g[3] = { 0.0f, -9.8f, 0.0f };

    switch (game_step(g, dt))
    {
    case GAME_STOP: goto_state(&st_stop); break;
    case GAME_GOAL: goto_state(&st_goal); break;
    case GAME_FALL: goto_state(&st_fall); break;
    }

    return 1;
}

static int roll_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_over) : 1;
}

/*---------------------------------------------------------------------------*/

static void goal_enter(void)
{
    hole_goal();
    menu_init(137, 4, 150);
    score_card("It's In!", c_green, c_green);
}

static void goal_leave(void)
{
    menu_free();
}

static void goal_paint(void)
{
    game_draw(0);
    menu_paint();
}

static int goal_click(int b, int d)
{
    if (b < 0 && d == 1)
    {
        if (hole_next())
            return goto_state(&st_next);
        else
            return goto_state(&st_over);
    }
    return 1;
}

static int goal_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_over) : 1;
}

/*---------------------------------------------------------------------------*/

static void stop_enter(void)
{
    hole_stop();
    menu_init(137, 4, 150);
    score_card("Score Card", c_white, c_white);
}

static void stop_leave(void)
{
    menu_free();
}

static void stop_paint(void)
{
    game_draw(0);
    menu_paint();
}

static int stop_click(int b, int d)
{
    if (b < 0 && d == 1)
    {
        if (hole_next())
            return goto_state(&st_next);
        else
            return goto_state(&st_over);
    }
    return 1;
}

static int stop_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_over) : 1;
}

/*---------------------------------------------------------------------------*/

static void fall_enter(void)
{
    hole_fall();
    menu_init(137, 4, 150);
    score_card("1 Stroke Penalty", c_black, c_red);
}

static void fall_leave(void)
{
    menu_free();
}

static void fall_paint(void)
{
    game_draw(0);
    menu_paint();
}

static int fall_click(int b, int d)
{
    if (b < 0 && d == 1)
    {
        if (hole_next())
            return goto_state(&st_next);
        else
            return goto_state(&st_over);
    }
    return 1;
}

static int fall_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_over) : 1;
}

/*---------------------------------------------------------------------------*/

static void over_enter(void)
{
    audio_music_fade(2.f);

    menu_init(137, 4, 150);
    score_card("Final Score", c_yellow, c_red);
}

static void over_leave(void)
{
    audio_music_stop();

    menu_free();
    hole_free();
}

static void over_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    menu_paint();
}

static int over_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int over_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_title) : 1;
}

/*---------------------------------------------------------------------------*/

struct state st_null = {
    null_enter,
    null_leave,
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
};

struct state st_conf = {
    conf_enter,
    conf_leave,
    conf_paint,
    NULL,
    conf_point,
    conf_click,
    conf_keybd,
};

struct state st_party = {
    party_enter,
    party_leave,
    party_paint,
    party_timer,
    party_point,
    party_click,
    party_keybd,
};

struct state st_next = {
    next_enter,
    next_leave,
    next_paint,
    NULL,
    NULL,
    next_click,
    next_keybd,
};

struct state st_flyby = {
    flyby_enter,
    flyby_leave,
    flyby_paint,
    flyby_timer,
    NULL,
    flyby_click,
    flyby_keybd,
};

struct state st_stroke = {
    stroke_enter,
    stroke_leave,
    stroke_paint,
    stroke_timer,
    stroke_point,
    stroke_click,
    stroke_keybd,
};

struct state st_roll = {
    roll_enter,
    roll_leave,
    roll_paint,
    roll_timer,
    NULL,
    NULL,
    roll_keybd,
};

struct state st_goal = {
    goal_enter,
    goal_leave,
    goal_paint,
    NULL,
    NULL,
    goal_click,
    goal_keybd,
};

struct state st_stop = {
    stop_enter,
    stop_leave,
    stop_paint,
    NULL,
    NULL,
    stop_click,
    stop_keybd,
};

struct state st_fall = {
    fall_enter,
    fall_leave,
    fall_paint,
    NULL,
    NULL,
    fall_click,
    fall_keybd,
};

struct state st_over = {
    over_enter,
    over_leave,
    over_paint,
    NULL,
    NULL,
    over_click,
    over_keybd,
};
