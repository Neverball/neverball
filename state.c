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
#include "back.h"
#include "menu.h"
#include "text.h"
#include "solid.h"
#include "state.h"
#include "level.h"
#include "set.h"
#include "image.h"
#include "audio.h"
#include "config.h"

FILE *record_fp;
FILE *replay_fp;

static double global_time;
static double replay_time;

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
    if (state && state->paint)
    {
        if (config_stereo())
        {
            glDrawBuffer(GL_BACK_LEFT);
            state->paint(+config_stereo());

            glDrawBuffer(GL_BACK_RIGHT);
            state->paint(-config_stereo());
        }
        else
            state->paint(0);
    }
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
#define TITLE_HELP 2
#define TITLE_DEMO 3
#define TITLE_CONF 4
#define TITLE_EXIT 5

#define STR_TITLE " Neverball "
#define STR_PLAY  " Play "
#define STR_HELP  " Help "
#define STR_DEMO  " Demo "
#define STR_CONF  " Options "
#define STR_EXIT  " Exit "

static int title_action(int i)
{
    switch (i)
    {
    case TITLE_PLAY: audio_play(AUD_MENU, 1.f); goto_state(&st_set);   break;
    case TITLE_HELP: audio_play(AUD_MENU, 1.f); goto_state(&st_help);  break;
    case TITLE_DEMO: audio_play(AUD_MENU, 1.f); goto_state(&st_demo);  break;
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

    menu_init(6, 6, TITLE_PLAY);

    menu_text(0,      0, lm + 2 * mh, c_yellow, c_red,   STR_TITLE, TXT_LRG);
    menu_text(TITLE_PLAY, 0, +1 * mh, c_white,  c_white, STR_PLAY,  TXT_MED);
    menu_text(TITLE_HELP, 0,  0 * mh, c_white,  c_white, STR_HELP,  TXT_MED);
    menu_text(TITLE_DEMO, 0, -1 * mh, c_white,  c_white, STR_DEMO,  TXT_MED);
    menu_text(TITLE_CONF, 0, -2 * mh, c_white,  c_white, STR_CONF,  TXT_MED);
    menu_text(TITLE_EXIT, 0, -3 * mh, c_white,  c_white, STR_EXIT,  TXT_MED);

    menu_item(0,      0, lm + 2 * mh, lw, lh);
    menu_item(TITLE_PLAY, 0, +1 * mh, mw, mh);
    menu_item(TITLE_HELP, 0,  0 * mh, mw, mh);
    menu_item(TITLE_DEMO, 0, -1 * mh, mw, mh);
    menu_item(TITLE_CONF, 0, -2 * mh, mw, mh);
    menu_item(TITLE_EXIT, 0, -3 * mh, mw, mh);

    menu_link(TITLE_PLAY,         -1, TITLE_HELP, -1, -1);
    menu_link(TITLE_HELP, TITLE_PLAY, TITLE_DEMO, -1, -1);
    menu_link(TITLE_DEMO, TITLE_HELP, TITLE_CONF, -1, -1);
    menu_link(TITLE_CONF, TITLE_DEMO, TITLE_EXIT, -1, -1);
    menu_link(TITLE_EXIT, TITLE_CONF,         -1, -1, -1);

    menu_stat(0,          -1);
    menu_stat(TITLE_PLAY, +1);
    menu_stat(TITLE_HELP,  0);
    menu_stat(TITLE_DEMO,  config_demo() ? 0 : -1);
    menu_stat(TITLE_CONF,  0);
    menu_stat(TITLE_EXIT,  0);

    set_init();
    set_goto(0);
    level_goto(0, 0, 0, 0);

    audio_music_play("bgm/title.ogg");

    SDL_ShowCursor(SDL_ENABLE);
}

static void title_leave(void)
{
    SDL_ShowCursor(SDL_DISABLE);

    set_free();
    menu_free();
    audio_music_play("bgm/inter.ogg");
}

static void title_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0, dy);
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

static void help_enter(void)
{
    const GLfloat *c0 = c_white;
    const GLfloat *c1 = c_yellow;
    int w, h;

    text_size("X", TXT_SML, &w, &h);
    w = config_w();

    menu_init(15, 1, -1);
    back_init("png/blues.png", config_geom());

    menu_item(0, 0, 0, 7 * w / 8, h * 14);

    menu_text(0, 0, +6 * h, c0, c0,
              "Move the mouse or joystick to tilt the floor,", TXT_SML);
    menu_text(1, 0, +5 * h, c0, c0,
              "causing the ball to roll.  Guide it to the goal", TXT_SML);
    menu_text(2, 0, +4 * h, c0, c0,
              "to finish the level.  Collect 100 coins to earn", TXT_SML);
    menu_text(3, 0, +3 * h, c0, c0,
              "an extra ball and save your progress.", TXT_SML);

    menu_text(4, -w / 4, +1 * h, c1, c1, "Spacebar", TXT_SML);
    menu_text(5, -w / 4,  0 * h, c1, c1, "Escape", TXT_SML);
    menu_text(6, -w / 4, -2 * h, c1, c1, "F1", TXT_SML);
    menu_text(7, -w / 4, -3 * h, c1, c1, "F2", TXT_SML);
    menu_text(8, -w / 4, -4 * h, c1, c1, "F3", TXT_SML);

    menu_text(9,  w / 8, +1 * h, c0, c0, "Pause / Release Pointer", TXT_SML);
    menu_text(10, w / 8,  0 * h, c0, c0, "Exit / Cancel Menu", TXT_SML);
    menu_text(11, w / 8, -2 * h, c0, c0, "Chase View", TXT_SML);
    menu_text(12, w / 8, -3 * h, c0, c0, "Lazy View", TXT_SML);
    menu_text(13, w / 8, -4 * h, c0, c0, "Manual View", TXT_SML);

    menu_text(14, 0, -6 * h, c0, c0,
              "Left and right mouse buttons rotate the view.", TXT_SML);

    SDL_ShowCursor(SDL_ENABLE);
}

static void help_leave(void)
{
    SDL_ShowCursor(SDL_DISABLE);
    menu_free();
    back_free();
}

static void help_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    config_push_persp(FOV, 0.1, 300.0);
    {
        back_draw(time_state());
    }
    config_pop_matrix();

    menu_paint();
}

static int help_click(int b, int d)
{
    return d ? goto_state(&st_title) : 1;
}

static int help_keybd(int c)
{
    return goto_state(&st_title);
}

static int help_buttn(int b, int d)
{
    return goto_state(&st_title);
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
    int w, h, w2, s = 1;
    char str[8];

    text_size("MMMMMMMMM", TXT_SML, &w, &h);
    w2 = w / 2;

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

    case CONF_SNDDN:
        config_set_sound(config_sound() - 1);
        sprintf(str, "%02d", config_sound());

        menu_text(33, +w2, -3 * h, c_yellow, c_red, str, TXT_SML);
        audio_play(AUD_BUMP, 1.f);
        break;
        
    case CONF_SNDUP:
        config_set_sound(config_sound() + 1);
        sprintf(str, "%02d", config_sound());

        menu_text(33, +w2, -3 * h, c_yellow, c_red, str, TXT_SML);
        audio_play(AUD_BUMP, 1.f);
        break;

    case CONF_MUSDN:
        config_set_music(config_music() - 1);
        sprintf(str, "%02d", config_music());

        menu_text(34, +w2, -4 * h, c_yellow, c_red, str, TXT_SML);
        break;
        
    case CONF_MUSUP:
        config_set_music(config_music() + 1);
        sprintf(str, "%02d", config_music());

        menu_text(34, +w2, -4 * h, c_yellow, c_red, str, TXT_SML);
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
    char snds[16];
    char muss[16];

    int w, w2, w4;
    int h, h2;
    
    /* Compute menu sizes from font sizes */

    text_size("MMMMMMMMM", TXT_SML, &w, &h);
    w2 = w / 2;
    w4 = w / 4;
    h2 = h / 2;

    sprintf(snds, "%02d", config_sound());
    sprintf(muss, "%02d", config_music());

    menu_init(36, 25, value);
    back_init("png/blues.png", config_geom());

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
    menu_stat(CONF_SNDDN, 0);
    menu_stat(CONF_SNDUP, 0);
    menu_stat(CONF_MUSDN, 0);
    menu_stat(CONF_MUSUP, 0);
    menu_stat(CONF_BACK,  0);

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
    menu_link(CONF_AUDHI, CONF_GEOHI, CONF_SNDDN, CONF_AUDHI, CONF_AUDLO);
    menu_link(CONF_AUDLO, CONF_GEOLO, CONF_SNDUP, CONF_AUDHI, CONF_AUDLO);
    menu_link(CONF_SNDDN, CONF_AUDHI, CONF_MUSDN, CONF_SNDDN, CONF_SNDUP);
    menu_link(CONF_SNDUP, CONF_AUDLO, CONF_MUSUP, CONF_SNDDN, CONF_SNDUP);
    menu_link(CONF_MUSDN, CONF_SNDDN, CONF_BACK,  CONF_MUSDN, CONF_MUSUP);
    menu_link(CONF_MUSUP, CONF_SNDUP, CONF_BACK,  CONF_MUSDN, CONF_MUSUP);
    menu_link(CONF_BACK,  CONF_MUSDN, CONF_BACK,  CONF_BACK,  CONF_BACK);

    value = -1;

    SDL_ShowCursor(SDL_ENABLE);
}

static void conf_leave(void)
{
    SDL_ShowCursor(SDL_DISABLE);

    config_store();
    menu_free();
    back_free();
}

static void conf_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    config_push_persp(FOV, 0.1, 300.0);
    {
        back_draw(time_state());
    }
    config_pop_matrix();

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

static GLuint shot_rect = 0;
static int    shot_set  = 0;

static int set_action(int i)
{
    if (i >= 0)
    {
        if (i == 0)
        {
            back_free();
            return goto_state(&st_title);
        }
        if (2 <= i && i <= 6)
        {
            back_free();
            set_goto(i - 2);
            return goto_state(&st_start);
        }
    }
    return 1;
}

static void set_describe(int i)
{
    int w, h, j, y, m = 7;

    char buf[STRMAX];
    const char *p = set_desc(i);

    text_size("0", TXT_SML, &w, &h);
    y = -2 * h;
    h =  3 * h / 2;

    while (p && m < 12)
    {
        memset(buf, 0, 256);
        j = 0;

        while (p && *p && *p != '\\')
            buf[j++] = *p++;

        if (j > 0) menu_text(m, 0, y, c_yellow, c_white, buf, TXT_SML);

        menu_item(m, 0, y, h * 15, h);
        menu_stat(m, -1);

        y -= h;
        m++;

        if (*p) p++;
    }
}

static void set_enter(void)
{
    int w;
    int h;
    int j;

    audio_play(AUD_START, 1.f);

    shot_set = 0;

    text_size("0", TXT_SML, &w, &h);
    j = h / 2;

    set_init();
    menu_init(12, 12, 0);
    back_init("png/blues.png", config_geom());

    /* Text elements */

    menu_text(0, -8 * h, 16 * j, c_white,  c_white,      "Back", TXT_SML);
    menu_text(1, +7 * h, 16 * j, c_yellow, c_red,   "Level Set", TXT_SML);
    menu_text(2, -5 * h, 13 * j, c_white,  c_white, set_name(0), TXT_SML);
    menu_text(3, -5 * h, 10 * j, c_white,  c_white, set_name(1), TXT_SML);
    menu_text(4, -5 * h,  7 * j, c_white,  c_white, set_name(2), TXT_SML);
    menu_text(5, -5 * h,  4 * j, c_white,  c_white, set_name(3), TXT_SML);
    menu_text(6, -5 * h,      j, c_white,  c_white, set_name(4), TXT_SML);

    /* Active items */

    menu_item(0, -8 * h, 16 * j,  4 * h, 3 * j);
    menu_item(1, +7 * h, 16 * j,  6 * h, 3 * j);
    menu_item(2, -5 * h, 13 * j, 10 * h, 3 * j);
    menu_item(3, -5 * h, 10 * j, 10 * h, 3 * j);
    menu_item(4, -5 * h,  7 * j, 10 * h, 3 * j);
    menu_item(5, -5 * h,  4 * j, 10 * h, 3 * j);
    menu_item(6, -5 * h,      j, 10 * h, 3 * j);

    /* Item state */

    menu_stat(0, 0);
    menu_stat(1, 0);
    menu_stat(2, set_exists(0) ? 0 : -1);
    menu_stat(3, set_exists(1) ? 0 : -1);
    menu_stat(4, set_exists(2) ? 0 : -1);
    menu_stat(5, set_exists(3) ? 0 : -1);
    menu_stat(6, set_exists(4) ? 0 : -1);

    /* Item linkings */

    menu_link(0,                     -1, set_exists(0) ? 2 : -1, -1, -1);
    menu_link(2,                      0, set_exists(1) ? 3 : -1, -1, -1);
    menu_link(3, set_exists(0) ? 2 : -1, set_exists(2) ? 4 : -1, -1, -1);
    menu_link(4, set_exists(1) ? 3 : -1, set_exists(3) ? 5 : -1, -1, -1);
    menu_link(5, set_exists(2) ? 4 : -1, set_exists(4) ? 6 : -1, -1, -1);
    menu_link(6, set_exists(3) ? 5 : -1,                     -1, -1, -1);

    set_describe(0);

    /* Position the set shot. */

    shot_rect = make_rect(config_w() / 2,
                          config_h() / 2 -      j / 2,
                          config_w() / 2 + 10 * h,
                          config_h() / 2 + 29 * j / 2);

    SDL_ShowCursor(SDL_ENABLE);
}

static void set_leave(void)
{
    if (glIsList(shot_rect))
        glDeleteLists(shot_rect, 1);

    SDL_ShowCursor(SDL_DISABLE);
    menu_free();
}

static void set_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    config_push_persp(FOV, 0.1, 300.0);
    {
        back_draw(time_state());
    }
    config_pop_matrix();

    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    config_push_ortho();
    {
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        set_shot(shot_set);
        glCallList(shot_rect);
    }
    config_pop_matrix();
    glPopAttrib();
    glPopAttrib();

    menu_paint();
}

static int set_point(int x, int y, int dx, int dy)
{
    int i;

    if ((i = menu_point(x, y)) >= 0 && shot_set != i - 2)
    {
        shot_set = i - 2;
        set_describe(shot_set);
    }

    return 1;
}

static int set_click(int b, int d)
{
    return (b < 0 && d == 1) ? set_action(menu_click()) : 1;
}

static int set_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int set_stick(int a, int v)
{
    menu_stick(a, v);
    return 1;
}

static int set_buttn(int b, int d)
{
    if (config_button_a(b) && d == 1) return set_action(menu_buttn());
    if (config_button_b(b) && d == 1) return goto_state(&st_title);
    if (config_button_X(b) && d == 1) return goto_state(&st_title);
    return 1;
}

/*---------------------------------------------------------------------------*/

static void score_menu(int i, int j, int k)
{
    const GLfloat *c0 = c_yellow;
    const GLfloat *c1 = c_white;
    int w, h;

    text_size("0", TXT_SML, &w, &h);

    menu_text(j,      -7 * h, 0, c1, c1, "Time:", TXT_SML);
    menu_text(j + 1,  +7 * h, 0, c1, c1, "Coins:", TXT_SML);

    menu_text(j + 2,  -2 * h, 0, c0, c1, level_time_s(i, -1), TXT_SML);
    menu_text(j + 3, +11 * h, 0, c0, c1, level_coin_c(i, -1), TXT_SML);

    menu_item(k,      -6 * h, 0, 12 * h - 4, 3 * h / 2);
    menu_item(k + 1,  +6 * h, 0, 12 * h - 4, 3 * h / 2);
}

static void highs_menu(int i, int j, int k)
{
    const GLfloat *c0 = c_yellow;
    const GLfloat *c1 = c_white;
    const GLfloat *c2 = c_red;
    int w, h;

    text_size("0", TXT_SML, &w, &h);

    menu_text(j,      -6*h,   -9*h/4, c0, c2, "Best Times", TXT_SML);
    menu_text(j +  1, +6*h,   -9*h/4, c0, c2, "Most Coins", TXT_SML);

    menu_text(j +  2, -21*h/2, -15*h/4, c0, c1, level_time_c(i,0), TXT_SML);
    menu_text(j +  3, -14*h/2, -15*h/4, c0, c1, level_time_n(i,0), TXT_SML);
    menu_text(j +  4,  -5*h/2, -15*h/4, c0, c1, level_time_s(i,0), TXT_SML);

    menu_text(j +  5, -21*h/2, -21*h/4, c0, c1, level_time_c(i,1), TXT_SML);
    menu_text(j +  6, -14*h/2, -21*h/4, c0, c1, level_time_n(i,1), TXT_SML);
    menu_text(j +  7,  -5*h/2, -21*h/4, c0, c1, level_time_s(i,1), TXT_SML);
    
    menu_text(j +  8, -21*h/2, -27*h/4, c0, c1, level_time_c(i,2), TXT_SML);
    menu_text(j +  9, -14*h/2, -27*h/4, c0, c1, level_time_n(i,2), TXT_SML);
    menu_text(j + 10,  -5*h/2, -27*h/4, c0, c1, level_time_s(i,2), TXT_SML);


    menu_text(j + 11,  +5*h/2, -15*h/4, c0, c1, level_coin_s(i,0), TXT_SML);
    menu_text(j + 12, +14*h/2, -15*h/4, c0, c1, level_coin_n(i,0), TXT_SML);
    menu_text(j + 13, +21*h/2, -15*h/4, c0, c1, level_coin_c(i,0), TXT_SML);

    menu_text(j + 14,  +5*h/2, -21*h/4, c0, c1, level_coin_s(i,1), TXT_SML);
    menu_text(j + 15, +14*h/2, -21*h/4, c0, c1, level_coin_n(i,1), TXT_SML);
    menu_text(j + 16, +21*h/2, -21*h/4, c0, c1, level_coin_c(i,1), TXT_SML);

    menu_text(j + 17,  +5*h/2, -27*h/4, c0, c1, level_coin_s(i,2), TXT_SML);
    menu_text(j + 18, +14*h/2, -27*h/4, c0, c1, level_coin_n(i,2), TXT_SML);
    menu_text(j + 19, +21*h/2, -27*h/4, c0, c1, level_coin_c(i,2), TXT_SML);

    menu_item(k,       -6*h, -18*h/4, 12 * h - 4, 12 * h / 2);
    menu_item(k +  1,  +6*h, -18*h/4, 12 * h - 4, 12 * h / 2);
}

/*---------------------------------------------------------------------------*/

#define START_BACK 0

static int shot_level = -1;

static int start_action(int i)
{
    if (i >= 0)
    {
        if (i == START_BACK)
        {
            back_free();
            return goto_state(&st_set);
        }
        else
        {
            back_free();
            level_goto(0, 0, 2, i);
            return goto_state(&st_level);
        }
    }
    return 1;
}

#define COL0(i) (level_exists(i) \
                 ? ((level_opened(i) ? c_white : c_yellow)) : c_black)
#define COL1(i) (level_exists(i) \
                 ? ((level_opened(i) ? c_white : c_red))    : c_black)
#define OPN(i)  (level_opened(i) ? i : -1)

static void start_enter(void)
{
    int i;
    int w;
    int h;
    int j;

    shot_level = 0;

    text_size("0", TXT_SML, &w, &h);
    j = h / 2;

    menu_init(47, 29, 0);
    back_init("png/blues.png", config_geom());

    /* Text elements */

    menu_text(0,  -8 * h, 16 * j, c_white, c_white,  "Set", TXT_SML);
    menu_text(1,  -9 * h, 13 * j, COL0( 1), COL1( 1), "01", TXT_SML);
    menu_text(2,  -7 * h, 13 * j, COL0( 2), COL1( 2), "02", TXT_SML);
    menu_text(3,  -5 * h, 13 * j, COL0( 3), COL1( 3), "03", TXT_SML);
    menu_text(4,  -3 * h, 13 * j, COL0( 4), COL1( 4), "04", TXT_SML);
    menu_text(5,  -1 * h, 13 * j, COL0( 5), COL1( 5), "05", TXT_SML);
    menu_text(6,  -9 * h, 10 * j, COL0( 6), COL1( 6), "06", TXT_SML);
    menu_text(7,  -7 * h, 10 * j, COL0( 7), COL1( 7), "07", TXT_SML);
    menu_text(8,  -5 * h, 10 * j, COL0( 8), COL1( 8), "08", TXT_SML);
    menu_text(9,  -3 * h, 10 * j, COL0( 9), COL1( 9), "09", TXT_SML);
    menu_text(10, -1 * h, 10 * j, COL0(10), COL1(10), "10", TXT_SML);
    menu_text(11, -9 * h,  7 * j, COL0(11), COL1(11), "11", TXT_SML);
    menu_text(12, -7 * h,  7 * j, COL0(12), COL1(12), "12", TXT_SML);
    menu_text(13, -5 * h,  7 * j, COL0(13), COL1(13), "13", TXT_SML);
    menu_text(14, -3 * h,  7 * j, COL0(14), COL1(14), "14", TXT_SML);
    menu_text(15, -1 * h,  7 * j, COL0(15), COL1(15), "15", TXT_SML);
    menu_text(16, -9 * h,  4 * j, COL0(16), COL1(16), "16", TXT_SML);
    menu_text(17, -7 * h,  4 * j, COL0(17), COL1(17), "17", TXT_SML);
    menu_text(18, -5 * h,  4 * j, COL0(18), COL1(18), "18", TXT_SML);
    menu_text(19, -3 * h,  4 * j, COL0(19), COL1(19), "19", TXT_SML);
    menu_text(20, -1 * h,  4 * j, COL0(20), COL1(20), "20", TXT_SML);
    menu_text(21, -9 * h,      j, COL0(21), COL1(21), "21", TXT_SML);
    menu_text(22, -7 * h,      j, COL0(22), COL1(22), "22", TXT_SML);
    menu_text(23, -5 * h,      j, COL0(23), COL1(23), "23", TXT_SML);
    menu_text(24, -3 * h,      j, COL0(24), COL1(24), "24", TXT_SML);
    menu_text(25, -1 * h,      j, COL0(25), COL1(25), "25", TXT_SML);
    menu_text(26,  8 * h, 16 * j, c_yellow, c_red, "Level", TXT_SML);

    highs_menu(0, 27, 27);

    /* Active items */

    menu_item(0,  -8 * h, 16 * j, 4 * h, 3 * j);
    menu_item(1,  -9 * h, 13 * j, 2 * h, 3 * j);
    menu_item(2,  -7 * h, 13 * j, 2 * h, 3 * j);
    menu_item(3,  -5 * h, 13 * j, 2 * h, 3 * j);
    menu_item(4,  -3 * h, 13 * j, 2 * h, 3 * j);
    menu_item(5,  -1 * h, 13 * j, 2 * h, 3 * j);
    menu_item(6,  -9 * h, 10 * j, 2 * h, 3 * j);
    menu_item(7,  -7 * h, 10 * j, 2 * h, 3 * j);
    menu_item(8,  -5 * h, 10 * j, 2 * h, 3 * j);
    menu_item(9,  -3 * h, 10 * j, 2 * h, 3 * j);
    menu_item(10, -1 * h, 10 * j, 2 * h, 3 * j);
    menu_item(11, -9 * h,  7 * j, 2 * h, 3 * j);
    menu_item(12, -7 * h,  7 * j, 2 * h, 3 * j);
    menu_item(13, -5 * h,  7 * j, 2 * h, 3 * j);
    menu_item(14, -3 * h,  7 * j, 2 * h, 3 * j);
    menu_item(15, -1 * h,  7 * j, 2 * h, 3 * j);
    menu_item(16, -9 * h,  4 * j, 2 * h, 3 * j);
    menu_item(17, -7 * h,  4 * j, 2 * h, 3 * j);
    menu_item(18, -5 * h,  4 * j, 2 * h, 3 * j);
    menu_item(19, -3 * h,  4 * j, 2 * h, 3 * j);
    menu_item(20, -1 * h,  4 * j, 2 * h, 3 * j);
    menu_item(21, -9 * h,      j, 2 * h, 3 * j);
    menu_item(22, -7 * h,      j, 2 * h, 3 * j);
    menu_item(23, -5 * h,      j, 2 * h, 3 * j);
    menu_item(24, -3 * h,      j, 2 * h, 3 * j);
    menu_item(25, -1 * h,      j, 2 * h, 3 * j);
    menu_item(26, +8 * h, 16 * j, 4 * h - 4,  3 * j); 

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

    shot_rect = make_rect(config_w() / 2,
                          config_h() / 2 -      j / 2,
                          config_w() / 2 + 10 * h,
                          config_h() / 2 + 29 * j / 2);

    SDL_ShowCursor(SDL_ENABLE);
}

static void start_leave(void)
{
    if (glIsList(shot_rect))
        glDeleteLists(shot_rect, 1);

    SDL_ShowCursor(SDL_DISABLE);
    menu_free();
}

static void start_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    config_push_persp(FOV, 0.1, 300.0);
    {
        back_draw(time_state());
    }
    config_pop_matrix();

    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    config_push_ortho();
    {
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        level_shot(shot_level);
        glCallList(shot_rect);
    }
    config_pop_matrix();
    glPopAttrib();
    glPopAttrib();

    menu_paint();
}

static int start_point(int x, int y, int dx, int dy)
{
    int i;

    if ((i = menu_point(x, y)) >= 0 && shot_level != i)
    {
        highs_menu(i, 27, 27);
        shot_level = i;
    }

    return 1;
}

static int start_click(int b, int d)
{
    return (b < 0 && d == 1) ? start_action(menu_click()) : 1;
}

static int start_keybd(int c)
{
    if (c == SDLK_ESCAPE)
        return goto_state(&st_title);

    if (c == SDLK_F12)
    {
        int i, n = curr_count();

        back_free();

        for (i = 1; i < n; i++)
            if (level_exists(i))
                level_snap(i);

        back_init("png/blues.png", config_geom());
    }

    return 1;
}

static int start_stick(int a, int v)
{
    int i;

    if ((i = menu_stick(a, v)) >= 0 && i != shot_level)
    {
        highs_menu(i, 27, 27);
        shot_level = i;
    }

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

    char buf[STRMAX], *p = curr_intro();

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
        memset(buf, 0, STRMAX);
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

    game_set_fly(1.0);
}

static void level_leave(void)
{
    menu_free();
}

static void level_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0, dy);
    hud_draw();
    menu_paint();
}

static int level_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_two) : 1;
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
    if (config_button_a(b) && d == 1) return goto_state(&st_two);
    if (config_button_X(b) && d == 1) return goto_state(&st_over);
    return 1;
}

/*---------------------------------------------------------------------------*/

static void poser_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(1, dy);
}

static int poser_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_level) : 1;
}

/*---------------------------------------------------------------------------*/

static void two_enter(void)
{
    int y = 1 * config_h() / 6;
    int h = 1 * config_w() / 6;
    int w = 2 * config_w() / 3;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_yellow, c_red, "Ready?", TXT_LRG);
    menu_item(0, 0, y, w, h);

    audio_music_fade(2.f);
    audio_play(AUD_READY, 1.f);
}

static void two_leave(void)
{
    menu_free();
}

static void two_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0, dy);
    hud_draw();
    menu_paint();
}

static int two_timer(double dt)
{
    double t = time_state();

    game_set_fly(1.0 - 0.5 * t);

    if (dt > 0.0 && t > 1.0)
        return goto_state(&st_one);

    return 1;
}

static int two_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_play) : 1;
}

static int two_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_over) : 1;
}

static int two_buttn(int b, int d)
{
    if (config_button_a(b) && d == 1) return goto_state(&st_play);
    if (config_button_X(b) && d == 1) return goto_state(&st_over);
    return 1;
}

/*---------------------------------------------------------------------------*/

static void one_enter(void)
{
    int y = 1 * config_h() / 6;
    int h = 1 * config_w() / 6;
    int w = 2 * config_w() / 3;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_yellow, c_red, "Set?", TXT_LRG);
    menu_item(0, 0, y, w, h);

    audio_play(AUD_SET, 1.f);
}

static void one_leave(void)
{
    menu_free();
}

static void one_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0, dy);
    hud_draw();
    menu_paint();
}

static int one_timer(double dt)
{
    double t = time_state();

    game_set_fly(0.5 - 0.5 * t);

    if (dt > 0.0 && t > 1.0)
        return goto_state(&st_play);

    return 1;
}

static int one_click(int b, int d)
{
    if (b < 0 && d == 1)
    {
        game_set_fly(0.0);
        return goto_state(&st_play);
    }
    return 1;
}

static int one_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_over) : 1;
}

static int one_buttn(int b, int d)
{
    if (config_button_a(b) && d == 1) return goto_state(&st_play);
    if (config_button_X(b) && d == 1) return goto_state(&st_over);
    return 1;
}

/*---------------------------------------------------------------------------*/

static int view_rotate = 0;

static void play_enter(void)
{
    char filename[STRMAX];

    int y = 1 * config_h() / 6;
    int h = 1 * config_w() / 6;
    int w = 2 * config_w() / 3;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_blue, c_green, "GO!", TXT_LRG);
    menu_item(0, 0, y, w, h);

    level_song();
    audio_play(AUD_GO, 1.f);

    game_set_fly(0.0);
    view_rotate = 0;

    if (config_home(filename, USER_REPLAY_FILE, STRMAX))
        if ((record_fp = fopen(filename, FMODE_WB)))
        {
            fputc(set_curr(),   record_fp);
            fputc(curr_score(), record_fp);
            fputc(curr_coins(), record_fp);
            fputc(curr_balls(), record_fp);
            fputc(curr_level(), record_fp);
        }
}

static void play_leave(void)
{
    menu_free();
}

static void play_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_draw(0, dy);
    hud_draw();

    if (time_state() < 1.0)
        menu_paint();
}

static int play_timer(double dt)
{
    static double at = 0;

    double g[3] = { 0.0, -9.8, 0.0 };

    at = (7 * at + dt) / 8;

    hud_step(at);
    game_set_rot(view_rotate);

    switch (game_step(g, at, 1))
    {
    case GAME_TIME: goto_state(&st_time); break;
    case GAME_GOAL: goto_state(&st_goal); break;
    case GAME_FALL: goto_state(&st_fall); break;
    }

    if (record_fp)
    {
        double_put(record_fp, &at);
        game_put(record_fp);
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

static void demo_enter(void)
{
    char filename[STRMAX];

    int y = 1 * config_h() / 6;
    int h = 1 * config_w() / 6;
    int w = 2 * config_w() / 3;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_blue, c_green, "Demo", TXT_LRG);
    menu_item(0, 0, y, w, h);

    global_time = -1.0;
    replay_time =  0.0;

    if (config_home(filename, USER_REPLAY_FILE, STRMAX))
        if ((replay_fp = fopen(filename, FMODE_RB)))
        {
            int l, b, c, s, n = fgetc(replay_fp);

            if (set_exists(n))
            {
                set_goto(n);

                s = fgetc(replay_fp);
                c = fgetc(replay_fp);
                b = fgetc(replay_fp);
                l = fgetc(replay_fp);

                if (level_exists(l))
                {
                    level_goto(s, c, b, l);
                    level_song();
                }
            }
        }

    game_set_fly(0.0);
}

static void demo_leave(void)
{
    if (replay_fp)
    {
        fclose(replay_fp);
        replay_fp = NULL;
    }
    menu_free();
}

static void demo_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_draw(0, dy);
    hud_draw();

    if (time_state() < 1.0)
        menu_paint();
}

static int demo_timer(double dt)
{
    double g[3] = { 0.0, -9.8, 0.0 };
    double t;
    int    b = 1;

    global_time += dt;

    if (replay_fp)
    {
        /* Spin or skip depending on how fast the demo wants to run. */

        while (replay_time < global_time && (b = double_get(replay_fp, &t)))
        {
            /* Play out current game state for particles, clock, etc. */

            hud_step(t);
            game_step(g, t, 1);

            replay_time += t;

            /* Load demo game state from file. */

            if (!game_get(replay_fp))
                goto_state(&st_omed);
        }
        if (!b) goto_state(&st_omed);
    }
    else goto_state(&st_omed);

    return 1;
}

static int demo_keybd(int c)
{
    if (c == SDLK_ESCAPE)
        goto_state(&st_omed);
    return 1;
}

static int demo_buttn(int b, int d)
{
    if (config_button_X(b) && d == 1)
        return goto_state(&st_omed);

    return 1;
}

/*---------------------------------------------------------------------------*/

static void goal_enter(void)
{
    int y = 1 * config_h() / 6;
    int h = 1 * config_w() / 6;
    int w = 2 * config_w() / 3;

    menu_init(25, 5, 99);
    menu_text(0, 0, y, c_blue, c_green, "GOAL!", TXT_LRG);
    menu_item(0, 0, y, w, h);

    score_menu(curr_level(), 1, 1);
    highs_menu(curr_level(), 5, 3);

    audio_music_fade(3.f);
    audio_play(AUD_GOAL, 1.f);
}

static void goal_leave(void)
{
    if (record_fp)
    {
        fclose(record_fp);
        record_fp = NULL;
    }
    menu_free();
    audio_music_play("bgm/inter.ogg");
}

static void goal_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0, dy);
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

static int goal_keybd(int c)
{
    if (c == SDLK_ESCAPE)
        goto_state(&st_over);
    return 1;
}

static int goal_timer(double dt)
{
    double g[3] = { 0.0, 9.8, 0.0 };
    double t = dt;

    if (time_state() < 1.0)
    {
        game_step(g, dt, 0);

        if (record_fp)
        {
            double_put(record_fp, &t);
            game_put(record_fp);
        }
    }
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
    int w, h, l = (int) strlen(player);

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

    menu_init(34, 32, 28);

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

    score_menu(curr_level(), 30, 30);

    /* Active elements. */

    menu_item(0,  0,     mh, mw * MAXNAM,        mh);
    menu_item(29, 0, 3 * mh, 8 * config_w() / 9, lh);

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

    audio_play(AUD_SCORE, 1.f);

    SDL_ShowCursor(SDL_ENABLE);
}

static void score_leave(void)
{
    SDL_ShowCursor(SDL_DISABLE);

    menu_free();
}

static void score_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0, dy);
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

    audio_music_fade(1.f);
    audio_play(AUD_FALL, 1.f);
}

static void fall_leave(void)
{
    if (record_fp)
    {
        fclose(record_fp);
        record_fp = NULL;
    }
    menu_free();
    audio_music_play("bgm/inter.ogg");
}

static void fall_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0, dy);
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

static int fall_keybd(int c)
{
    if (c == SDLK_ESCAPE)
        goto_state(&st_over);
    return 1;
}

static int fall_timer(double dt)
{
    double g[3] = { 0.0, -9.8, 0.0 };
    double t = dt;

    if (time_state() < 2.0)
    {
        game_step(g, dt, 0);

        if (record_fp)
        {
            double_put(record_fp, &t);
            game_put(record_fp);
        }
    }
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

    audio_music_fade(1.f);
    audio_play(AUD_TIME,  1.f);
}

static void time_leave(void)
{
    if (record_fp)
    {
        fclose(record_fp);
        record_fp = NULL;
    }
    menu_free();
    audio_music_play("bgm/inter.ogg");
}

static void time_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0, dy);
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
    double t = dt;

    if (time_state() < 2.0)
    {
        game_step(g, dt, 0);

        if (record_fp)
        {
            double_put(record_fp, &t);
            game_put(record_fp);
        }
    }
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

static void omed_enter(void)
{
    int y = 1 * config_h() / 6;
    int h = 1 * config_w() / 6;
    int w = 7 * config_w() / 8;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_black, c_red, "Demo Ends", TXT_LRG);
    menu_item(0, 0, y, w, h);

    audio_music_fade(3.f);
}

static void omed_leave(void)
{
    audio_music_stop();
    menu_free();
    set_free();
}

static void omed_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0, dy);
    menu_paint();
}

static int omed_timer(double dt)
{
    return (dt > 0.0 && time_state() > 3.0) ? goto_state(&st_title) : 1;
}

static int omed_keybd(int c)
{
    return (c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int omed_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_title) : 1;
}

static int omed_buttn(int b, int d)
{
    if (config_button_a(b) && d == 1) goto_state(&st_title);
    if (config_button_b(b) && d == 1) goto_state(&st_title);
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

    audio_music_stop();
    audio_play(AUD_OVER, 1.f);
}

static void over_leave(void)
{
    if (record_fp)
    {
        fclose(record_fp);
        record_fp = NULL;
    }
    menu_free();
    set_free();
}

static void over_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0, dy);
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

#define STR_DONE " Congrats! "
#define STR_THNX " That's it for this set. "
#define STR_MORE " Move on to the next one. "

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
    set_free();
}

static void done_paint(double dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game_draw(0, dy);
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

struct state st_help = {
    help_enter,
    help_leave,
    help_paint,
    NULL,
    NULL,
    help_click,
    help_keybd,
    NULL,
    help_buttn,
};

struct state st_set = {
    set_enter,
    set_leave,
    set_paint,
    NULL,
    set_point,
    set_click,
    set_keybd,
    set_stick,
    set_buttn,
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

struct state st_two = {
    two_enter,
    two_leave,
    two_paint,
    two_timer,
    NULL,
    two_click,
    two_keybd,
    NULL,
    two_buttn,
};

struct state st_one = {
    one_enter,
    one_leave,
    one_paint,
    one_timer,
    NULL,
    one_click,
    one_keybd,
    NULL,
    one_buttn,
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

struct state st_demo = {
    demo_enter,
    demo_leave,
    demo_paint,
    demo_timer,
    NULL,
    NULL,
    demo_keybd,
    NULL,
    demo_buttn,
};

struct state st_goal = {
    goal_enter,
    goal_leave,
    goal_paint,
    goal_timer,
    NULL,
    goal_click,
    goal_keybd,
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
    fall_keybd,
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

struct state st_omed = {
    omed_enter,
    omed_leave,
    omed_paint,
    omed_timer,
    NULL,
    omed_click,
    omed_keybd,
    NULL,
    omed_buttn,
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

