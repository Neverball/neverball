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
#include "gui.h"
#include "vec3.h"
#include "geom.h"
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

static float global_time;
static float replay_time;

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

    if (state && state->pointer == 0)
        SDL_ShowCursor(SDL_ENABLE);

    state      = st;
    state_time =  0;

    if (state && state->pointer == 0)
        SDL_ShowCursor(SDL_DISABLE);

    if (state && state->enter)
        state->enter();

    return 1;
}

/*---------------------------------------------------------------------------*/

void st_paint(void)
{
    int s = config_get(CONFIG_STEREO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if (state && state->paint)
    {
        if (s)
        {
            glDrawBuffer(GL_BACK_LEFT);
            state->paint((float) (+s));

            glDrawBuffer(GL_BACK_RIGHT);
            state->paint((float) (-s));
        }
        else
            state->paint(0.0f);
    }
}

void st_timer(float t)
{
    state_time += t;

    if (state && state->timer)
        state->timer(t);
}

void st_point(int x, int y, int dx, int dy)
{
    if (state && state->point)
        state->point(x, y, dx, dy);
}

void st_stick(int a, int k)
{
    if (state && state->stick)
        state->stick(a, k);
}

/*---------------------------------------------------------------------------*/

int st_click(int b, int d)
{
    return (state && state->click) ? state->click(b, d) : 1;
}

int st_keybd(int c, int d)
{
    return (state && state->keybd) ? state->keybd(c, d) : 1;
}

int st_buttn(int b, int d)
{
    return (state && state->buttn) ? state->buttn(b, d) : 1;
}

/*---------------------------------------------------------------------------*/

static void null_enter(void)
{
    hud_free();
    gui_free();
    swch_free();
    jump_free();
    goal_free();
    coin_free();
    ball_free();
    text_free();
}

static void null_leave(void)
{
    int h = config_get(CONFIG_HEIGHT);
    int g = config_get(CONFIG_GEOMETRY);

    text_init(h);
    ball_init(g);
    coin_init(g);
    goal_init(g);
    jump_init(g);
    swch_init(g);
    gui_init();
    hud_init();
}

/*---------------------------------------------------------------------------*/

static int title_id;

#define TITLE_PLAY 1
#define TITLE_HELP 2
#define TITLE_DEMO 3
#define TITLE_CONF 4
#define TITLE_EXIT 5

static int title_action(int i)
{
    switch (i)
    {
    case TITLE_PLAY: audio_play(AUD_MENU, 1.f); return goto_state(&st_set);
    case TITLE_HELP: audio_play(AUD_MENU, 1.f); return goto_state(&st_help);
    case TITLE_DEMO: audio_play(AUD_MENU, 1.f); return goto_state(&st_demo);
    case TITLE_CONF: audio_play(AUD_MENU, 1.f); return goto_state(&st_conf);
    case TITLE_EXIT: return 0;
    }
    return 1;
}

static void title_enter(void)
{
    int id, jd;

    /* Build the title GUI. */

    if ((title_id = gui_vstack(0)))
    {
        gui_label(title_id, "Neverball", GUI_LRG, GUI_ALL, 0, 0);
        gui_space(title_id);

        if ((id = gui_harray(title_id)))
        {
            gui_filler(id);

            if ((jd = gui_varray(id)))
            {
                gui_start(jd, "Play",    GUI_MED, TITLE_PLAY, 1);
                gui_state(jd, "Help",    GUI_MED, TITLE_HELP, 0);
                gui_state(jd, "Demo",    GUI_MED, TITLE_DEMO, 0);
                gui_state(jd, "Options", GUI_MED, TITLE_CONF, 0);
                gui_state(jd, "Exit",    GUI_MED, TITLE_EXIT, 0);
            }

            gui_filler(id);
        }

        gui_layout(title_id, 0, 0);
    }

    /* Initialize the first level of the first set for display. */

    set_init();
    set_goto(0);
    level_goto(0, 0, 0, 0);

    /* Start the title screen music. */

    audio_music_play("bgm/title.ogg");
}

static void title_leave(void)
{
    audio_music_play("bgm/inter.ogg");

    set_free();
    gui_delete(title_id);
}

static void title_paint(float dy)
{
    game_draw(0, dy);
    gui_paint(title_id);
}

static void title_timer(float dt)
{
    game_set_fly(fcosf(time_state() / 10.f));
    gui_timer(title_id, dt);
}

static void title_point(int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(title_id, x, y), 1.2f);
}

static void title_stick(int a, int v)
{
    if (config_tst(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(title_id, v, 0), 1.2f);
    if (config_tst(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(title_id, 0, v), 1.2f);
}

static int title_click(int b, int d)
{
    if (d && b < 0)
        return title_action(gui_token(gui_click()));
    return 1;
}

static int title_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? 0 : 1;
}

static int title_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return title_action(gui_token(gui_click()));
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void help_enter(void)
{
    const GLfloat *c0 = c_white;
    const GLfloat *c1 = c_ylw;
    int w, h;

    text_size("X", TXT_SML, &w, &h);
    w = config_get(CONFIG_WIDTH);

    menu_init(15, 1, -1);
    back_init("png/blues.png", config_get(CONFIG_GEOMETRY));

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
    menu_text(6, -w / 4, -2 * h, c1, c1,
              SDL_GetKeyName(config_get(CONFIG_KEY_CAMERA_1)), TXT_SML);
    menu_text(7, -w / 4, -3 * h, c1, c1,
              SDL_GetKeyName(config_get(CONFIG_KEY_CAMERA_2)), TXT_SML);
    menu_text(8, -w / 4, -4 * h, c1, c1,
              SDL_GetKeyName(config_get(CONFIG_KEY_CAMERA_3)), TXT_SML);

    menu_text(9,  w / 8, +1 * h, c0, c0, "Pause / Release Pointer", TXT_SML);
    menu_text(10, w / 8,  0 * h, c0, c0, "Exit / Cancel Menu", TXT_SML);
    menu_text(11, w / 8, -2 * h, c0, c0, "Chase View", TXT_SML);
    menu_text(12, w / 8, -3 * h, c0, c0, "Lazy View", TXT_SML);
    menu_text(13, w / 8, -4 * h, c0, c0, "Manual View", TXT_SML);

    menu_text(14, 0, -6 * h, c0, c0,
              "Left and right mouse buttons rotate the view.", TXT_SML);
}

static void help_leave(void)
{
    menu_free();
    back_free();
}

static void help_paint(float dy)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    config_push_persp(FOV, 0.1f, FAR_DIST);
    {
        back_draw(0, time_state());
    }
    config_pop_matrix();

    menu_paint();
}

static int help_click(int b, int d)
{
    return d ? goto_state(&st_title) : 1;
}

static int help_keybd(int c, int d)
{
    return goto_state(&st_title);
}

static int help_buttn(int b, int d)
{
    return goto_state(&st_title);
}

/*---------------------------------------------------------------------------*/

#define CONF_FULL  1
#define CONF_WIN   2
#define CONF_16x12 3
#define CONF_12x10 4
#define CONF_10x7  5
#define CONF_8x6   6
#define CONF_6x4   7
#define CONF_TEXHI 8
#define CONF_TEXLO 9
#define CONF_GEOHI 10
#define CONF_GEOLO 11
#define CONF_REFON 12
#define CONF_REFOF 13
#define CONF_AUDHI 14
#define CONF_AUDLO 15
#define CONF_SNDDN 16
#define CONF_SND   17
#define CONF_SNDUP 18
#define CONF_MUSDN 19
#define CONF_MUS   20
#define CONF_MUSUP 21
#define CONF_BACK  22

static int conf_id;
static int conf_snd_id;
static int conf_mus_id;

static int conf_action(int i)
{
    int f   = config_get(CONFIG_FULLSCREEN);
    int w   = config_get(CONFIG_WIDTH);
    int h   = config_get(CONFIG_HEIGHT);
    int snd = config_get(CONFIG_SOUND_VOLUME);
    int mus = config_get(CONFIG_MUSIC_VOLUME);
    int s   = 1;

    switch (i)
    {
    case CONF_FULL:
        goto_state(&st_null);
        s = config_mode(1, w, h);
        goto_state(&st_conf);
        break;

    case CONF_WIN:
        goto_state(&st_null);
        s = config_mode(0, w, h);
        goto_state(&st_conf);
        break;
        
    case CONF_16x12:
        goto_state(&st_null);
        s = config_mode(f, 1600, 1200);
        goto_state(&st_conf);
        break;

    case CONF_12x10:
        goto_state(&st_null);
        s = config_mode(f, 1280, 1024);
        goto_state(&st_conf);
        break;

    case CONF_10x7:
        goto_state(&st_null);
        s = config_mode(f, 1024, 768);
        goto_state(&st_conf);
        break;
            
    case CONF_8x6:
        goto_state(&st_null);
        s = config_mode(f, 800, 600);
        goto_state(&st_conf);
        break;

    case CONF_6x4:
        goto_state(&st_null);
        s = config_mode(f, 640, 480);
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

    case CONF_REFON:
        goto_state(&st_null);
        config_set(CONFIG_REFLECTION, 1);
        goto_state(&st_conf);
        break;

    case CONF_REFOF:
        goto_state(&st_null);
        config_set(CONFIG_REFLECTION, 0);
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
        if (--snd >= 0)
        {
            gui_set_count(conf_snd_id, snd, GUI_SML);
            config_set(CONFIG_SOUND_VOLUME, snd);
            audio_volume(snd, mus);
            audio_play(AUD_BUMP, 1.f);
        }
        break;
        
    case CONF_SNDUP:
        if (++snd <= 10)
        {
            gui_set_count(conf_snd_id, snd, GUI_SML);
            config_set(CONFIG_SOUND_VOLUME, snd);
            audio_volume(snd, mus);
            audio_play(AUD_BUMP, 1.f);
        }
        break;

    case CONF_MUSDN:
        if (--mus >= 0)
        {
            gui_set_count(conf_mus_id, mus, GUI_SML);
            config_set(CONFIG_MUSIC_VOLUME, mus);
            audio_volume(snd, mus);
        }
        break;
        
    case CONF_MUSUP:
        if (++mus <= 10)
        {
            gui_set_count(conf_mus_id, mus, GUI_SML);
            config_set(CONFIG_MUSIC_VOLUME, mus);
            audio_volume(snd, mus);
        }
        break;

    case CONF_BACK:
        goto_state(&st_title);
        break;
    }

    return s;
}

static void conf_enter(void)
{
    int id;

    if ((id = gui_varray(0)))
    {
        int a = config_tst(CONFIG_WIDTH, 1600);
        int b = config_tst(CONFIG_WIDTH, 1280);
        int c = config_tst(CONFIG_WIDTH, 1024);
        int d = config_tst(CONFIG_WIDTH,  800);
        int e = config_tst(CONFIG_WIDTH,  640);

        gui_state(id, "1600 x 1200", GUI_SML, CONF_16x12, a);
        gui_state(id, "1280 x 1024", GUI_SML, CONF_12x10, b);
        gui_state(id, "1024 x 768",  GUI_SML, CONF_10x7,  c);
        gui_state(id, "800 x 600",   GUI_SML, CONF_8x6,   d);
        gui_state(id, "640 x 480",   GUI_SML, CONF_6x4,   e);
    }

    conf_id = id;
    gui_layout(id, 0, 0);


    /*
    const float *R = c_red;
    const float *Y = c_ylw;

    int id;
    int jd;

    conf_id = gui_harray();

    jd = gui_varray();
    gui_insert(jd, gui_start("Back", 0, 0, GUI_SML, 0, CONF_BACK));
    {
        int a = config_tst(CONFIG_WIDTH, 1600);
        int b = config_tst(CONFIG_WIDTH, 1280);
        int c = config_tst(CONFIG_WIDTH, 1024);
        int d = config_tst(CONFIG_WIDTH,  800);
        int e = config_tst(CONFIG_WIDTH,  640);

        gui_insert(jd, gui_label("1600 x 1200", 0, 0, GUI_SML, a, CONF_16x12));
        gui_insert(jd, gui_label("1280 x 1024", 0, 0, GUI_SML, b, CONF_12x10));
        gui_insert(jd, gui_label("1024 x 768",  0, 0, GUI_SML, c, CONF_10x7));
        gui_insert(jd, gui_label("800 x 600",   0, 0, GUI_SML, d, CONF_8x6));
        gui_insert(jd, gui_label("640 x 480",   0, 0, GUI_SML, e, CONF_6x4));
    }

    id = gui_harray();
    {
        int h = config_tst(CONFIG_TEXTURES, 1);
        int l = config_tst(CONFIG_TEXTURES, 2);

        gui_insert(id, gui_label("Low",  0, 0, GUI_SML, l, CONF_TEXLO));
        gui_insert(id, gui_label("High", 0, 0, GUI_SML, h, CONF_TEXHI));
    }
    gui_insert(jd, id);
    id = gui_harray();
    {
        int h = config_tst(CONFIG_GEOMETRY, 1);
        int l = config_tst(CONFIG_GEOMETRY, 0);

        gui_insert(id, gui_label("Low",  0, 0, GUI_SML, l, CONF_GEOLO));
        gui_insert(id, gui_label("High", 0, 0, GUI_SML, h, CONF_GEOHI));
    }
    gui_insert(jd, id);
    id = gui_harray();
    {
        int h = config_tst(CONFIG_REFLECTION, 1);
        int l = config_tst(CONFIG_REFLECTION, 0);

        gui_insert(id, gui_label("Off",  0, 0, GUI_SML, l, CONF_REFOF));
        gui_insert(id, gui_label("On",   0, 0, GUI_SML, h, CONF_REFON));
    }
    gui_insert(jd, id);
    id = gui_harray();
    {
        int h = config_tst(CONFIG_AUDIO_RATE, 44100);
        int l = config_tst(CONFIG_AUDIO_RATE, 22050);

        gui_insert(id, gui_label("Low",  0, 0, GUI_SML, l, CONF_AUDLO));
        gui_insert(id, gui_label("High", 0, 0, GUI_SML, h, CONF_AUDHI));
    }
    gui_insert(jd, id);
    id = gui_harray();
    {
        int v = config_get(CONFIG_SOUND_VOLUME);

        conf_snd_id  = gui_count(v,   Y, R, GUI_SML, 0, CONF_SND);
        gui_insert(id, gui_label(">", 0, 0, GUI_SML, 0, CONF_SNDUP));
        gui_insert(id, conf_snd_id); 
        gui_insert(id, gui_label("<", 0, 0, GUI_SML, 0, CONF_SNDDN));
    }
    gui_insert(jd, id);
    id = gui_harray();
    {
        int v = config_get(CONFIG_MUSIC_VOLUME);

        conf_mus_id  = gui_count(v,   Y, R, GUI_SML, 0, CONF_MUS);
        gui_insert(id, gui_label(">", 0, 0, GUI_SML, 0, CONF_MUSUP));
        gui_insert(id, conf_mus_id);
        gui_insert(id, gui_label("<", 0, 0, GUI_SML, 0, CONF_MUSDN));
    }
    gui_insert(jd, id);
    gui_insert(conf_id, jd);

    id = gui_varray();
    {
        int f = config_tst(CONFIG_FULLSCREEN, 1);
        int w = config_tst(CONFIG_FULLSCREEN, 0);

        gui_insert(id, gui_label("Options",    Y, R, GUI_MED, -1, -1));
        gui_insert(id, gui_label("Fullscreen", 0, 0, GUI_SML,  f, CONF_FULL));
        gui_insert(id, gui_label("Window",     0, 0, GUI_SML,  w, CONF_WIN));
        gui_insert(id, gui_label(" ",          Y, R, GUI_SML, -1, -1));
        gui_insert(id, gui_label("Textures",   Y, R, GUI_SML, -1, -1));
        gui_insert(id, gui_label("Geometry",   Y, R, GUI_SML, -1, -1));
        gui_insert(id, gui_label("Reflection", Y, R, GUI_SML, -1, -1));
        gui_insert(id, gui_label("Audio",      Y, R, GUI_SML, -1, -1));
        gui_insert(id, gui_label("Sound",      Y, R, GUI_SML, -1, -1));
        gui_insert(id, gui_label("Music",      Y, R, GUI_SML, -1, -1));
        gui_insert(id, gui_label(" ",          Y, R, GUI_SML, -1, -1));
    }
    gui_insert(conf_id, id);
    gui_layout(conf_id, 0, 0);
    */
    /* Initialize the background. */

    back_init("png/blues.png", config_get(CONFIG_GEOMETRY));

    /* Start the title screen music. */

    audio_music_play("bgm/title.ogg");
}

static void conf_leave(void)
{
    gui_delete(conf_id);
    back_free();
}

static void conf_paint(float dy)
{
    config_push_persp(FOV, 0.1f, FAR_DIST);
    {
        back_draw(0, time_state());
    }
    config_pop_matrix();

    gui_paint(conf_id);
}

static void conf_timer(float dt)
{
    gui_timer(conf_id, dt);
}

static void conf_point(int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(conf_id, x, y), 1.2f);
}

static void conf_stick(int a, int v)
{
    if (config_tst(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(conf_id, v, 0), 1.2f);
    if (config_tst(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(conf_id, 0, v), 1.2f);
}

static int conf_click(int b, int d)
{
    if (b < 0 && d == 1)
        return conf_action(gui_token(gui_click()));
    return 1;
}

static int conf_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int conf_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return conf_action(gui_token(gui_click()));
        if (config_tst(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_title);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_title);
    }
    return 1;
}

#ifdef SNIP
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
#define CONF_REFON 11
#define CONF_REFOF 12
#define CONF_AUDHI 13
#define CONF_AUDLO 14
#define CONF_SNDDN 15
#define CONF_SNDUP 16
#define CONF_MUSDN 17
#define CONF_MUSUP 18
#define CONF_BACK  19

static int conf_value = CONF_FULL;

static int conf_action(int i)
{
    int w, h, w2, s = 1;
    char str[8];

    int snd = config_get(CONFIG_SOUND_VOLUME);
    int mus = config_get(CONFIG_MUSIC_VOLUME);

    conf_value = i;

    text_size("MMMMMMMMM", TXT_SML, &w, &h);
    w2 = w / 2;

    switch (i)
    {
    case CONF_FULL:
        goto_state(&st_null);
        s = config_mode(1, config_get(CONFIG_WIDTH), config_get(CONFIG_HEIGHT));
        goto_state(&st_conf);
        break;

    case CONF_WIN:
        goto_state(&st_null);
        s = config_mode(0, config_get(CONFIG_WIDTH), config_get(CONFIG_HEIGHT));
        goto_state(&st_conf);
        break;
        
    case CONF_16x12:
        goto_state(&st_null);
        s = config_mode(config_get(CONFIG_FULLSCREEN), 1600, 1200);
        goto_state(&st_conf);
        break;

    case CONF_12x10:
        goto_state(&st_null);
        s = config_mode(config_get(CONFIG_FULLSCREEN), 1280, 1024);
        goto_state(&st_conf);
        break;

    case CONF_10x7:
        goto_state(&st_null);
        s = config_mode(config_get(CONFIG_FULLSCREEN), 1024, 768);
        goto_state(&st_conf);
        break;
            
    case CONF_8x6:
        goto_state(&st_null);
        s = config_mode(config_get(CONFIG_FULLSCREEN), 800, 600);
        goto_state(&st_conf);
        break;

    case CONF_6x4:
        goto_state(&st_null);
        s = config_mode(config_get(CONFIG_FULLSCREEN), 640, 480);
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

    case CONF_REFON:
        goto_state(&st_null);
        config_set(CONFIG_REFLECTION, 1);
        goto_state(&st_conf);
        break;

    case CONF_REFOF:
        goto_state(&st_null);
        config_set(CONFIG_REFLECTION, 0);
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
        if (snd > 0)
        {
            config_set(CONFIG_SOUND_VOLUME, snd - 1);
            audio_volume(snd - 1, mus);
        
            sprintf(str, "%02d", snd - 1);
            menu_text(36, +w2, -4 * h, c_ylw, c_red, str, TXT_SML);
            audio_play(AUD_BUMP, 1.f);
        }
        break;
        
    case CONF_SNDUP:
        if (snd < 10)
        {
            config_set(CONFIG_SOUND_VOLUME, snd + 1);
            audio_volume(snd + 1, mus);
        
            sprintf(str, "%02d", snd + 1);
            menu_text(36, +w2, -4 * h, c_ylw, c_red, str, TXT_SML);
            audio_play(AUD_BUMP, 1.f);
        }
        break;

    case CONF_MUSDN:
        if (mus > 0)
        {
            config_set(CONFIG_MUSIC_VOLUME, mus - 1);
            audio_volume(snd, mus - 1);
        
            sprintf(str, "%02d", mus - 1);
            menu_text(37, +w2, -5 * h, c_ylw, c_red, str, TXT_SML);
        }
        break;
        
    case CONF_MUSUP:
        if (mus < 10)
        {
            config_set(CONFIG_MUSIC_VOLUME, mus + 1);
            audio_volume(snd, mus + 1);
        
            sprintf(str, "%02d", mus + 1);
            menu_text(37, +w2, -5 * h, c_ylw, c_red, str, TXT_SML);
        }
        break;

    case CONF_BACK:
        goto_state(&st_title);
        break;
    }

    if (s) config_save();
    return s;
}

static void conf_enter(void)
{
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

    menu_init(39, 28, conf_value);
    back_init("png/blues.png", config_get(CONFIG_GEOMETRY));

    /* Text elements */

    menu_text(0,  -w2, +h2 + 4 * h, c_ylw, c_red,   "Options",    TXT_MED);
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
    menu_text(18, -w2,       0 * h, c_ylw, c_red,   "Textures",   TXT_SML);
    menu_text(19, -w2,      -1 * h, c_ylw, c_red,   "Geometry",   TXT_SML);
    menu_text(20, -w2,      -2 * h, c_ylw, c_red,   "Reflection", TXT_SML);
    menu_text(21, -w2,      -3 * h, c_ylw, c_red,   "Audio",      TXT_SML);
    menu_text(22, -w2,      -4 * h, c_ylw, c_red,   "Sound Vol",  TXT_SML);
    menu_text(23, -w2,      -5 * h, c_ylw, c_red,   "Music Vol",  TXT_SML);
    menu_text(24, +w2 - w4,  0 * h, c_white,  c_white, "High",       TXT_SML);
    menu_text(25, +w2 - w4, -1 * h, c_white,  c_white, "High",       TXT_SML);
    menu_text(26, +w2 - w4, -2 * h, c_white,  c_white, "On",         TXT_SML);
    menu_text(27, +w2 - w4, -3 * h, c_white,  c_white, "High",       TXT_SML);
    menu_text(28, +w2 - w4, -4 * h, c_white,  c_white, "<",          TXT_SML);
    menu_text(29, +w2 - w4, -5 * h, c_white,  c_white, "<",          TXT_SML);
    menu_text(30, +w2 + w4,  0 * h, c_white,  c_white, "Low",        TXT_SML);
    menu_text(31, +w2 + w4, -1 * h, c_white,  c_white, "Low",        TXT_SML);
    menu_text(32, +w2 + w4, -2 * h, c_white,  c_white, "Off",        TXT_SML);
    menu_text(33, +w2 + w4, -3 * h, c_white,  c_white, "Low",        TXT_SML);
    menu_text(34, +w2 + w4, -4 * h, c_white,  c_white, ">",          TXT_SML);
    menu_text(35, +w2 + w4, -5 * h, c_white,  c_white, ">",          TXT_SML);
    menu_text(36, +w2,      -4 * h, c_ylw, c_red,   snds,         TXT_SML);
    menu_text(37, +w2,      -5 * h, c_ylw, c_red,   muss,         TXT_SML);
    menu_text(38, +w2,      -6 * h, c_white,  c_white, "Back",       TXT_SML);

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
    menu_item(CONF_REFON, +w2 - w4, -2 * h, w2, h);
    menu_item(CONF_REFOF, +w2 + w4, -2 * h, w2, h);
    menu_item(CONF_AUDHI, +w2 - w4, -3 * h, w2, h);
    menu_item(CONF_AUDLO, +w2 + w4, -3 * h, w2, h);
    menu_item(CONF_SNDDN, +w2 - w4, -4 * h, w2, h);
    menu_item(CONF_SNDUP, +w2 + w4, -4 * h, w2, h);
    menu_item(CONF_MUSDN, +w2 - w4, -5 * h, w2, h);
    menu_item(CONF_MUSUP, +w2 + w4, -5 * h, w2, h);
    menu_item(CONF_BACK,  +w2,      -6 * h, w,  h);

    /* Inactive label padding */

    menu_item(20, -w2, +5 * h, w,  h);
    menu_item(21, -w2, +4 * h, w,  h);
    menu_item(22, -w2,  0 * h, w,  h);
    menu_item(23, -w2, -1 * h, w,  h);
    menu_item(24, -w2, -2 * h, w,  h);
    menu_item(25, -w2, -3 * h, w,  h);
    menu_item(26, -w2, -4 * h, w,  h);
    menu_item(27, -w2, -5 * h, w,  h);

    /* Item state */

    menu_stat(CONF_FULL,  (config_get(CONFIG_FULLSCREEN))          ? 1 : 0);
    menu_stat(CONF_WIN,   (config_get(CONFIG_FULLSCREEN))          ? 0 : 1);
    menu_stat(CONF_16x12, (config_get(CONFIG_WIDTH)      == 1600)  ? 1 : 0);
    menu_stat(CONF_12x10, (config_get(CONFIG_WIDTH)      == 1280)  ? 1 : 0);
    menu_stat(CONF_10x7,  (config_get(CONFIG_WIDTH)      == 1024)  ? 1 : 0);
    menu_stat(CONF_8x6,   (config_get(CONFIG_WIDTH)      ==  800)  ? 1 : 0);
    menu_stat(CONF_6x4,   (config_get(CONFIG_WIDTH)      ==  640)  ? 1 : 0);
    menu_stat(CONF_TEXHI, (config_get(CONFIG_TEXTURES)   == 1)     ? 1 : 0);
    menu_stat(CONF_TEXLO, (config_get(CONFIG_TEXTURES)   == 2)     ? 1 : 0);
    menu_stat(CONF_GEOHI, (config_get(CONFIG_GEOMETRY)   == 1)     ? 1 : 0);
    menu_stat(CONF_GEOLO, (config_get(CONFIG_GEOMETRY)   == 0)     ? 1 : 0);
    menu_stat(CONF_REFON, (config_get(CONFIG_REFLECTION) == 1)     ? 1 : 0);
    menu_stat(CONF_REFOF, (config_get(CONFIG_REFLECTION) == 0)     ? 1 : 0);
    menu_stat(CONF_AUDHI, (config_get(CONFIG_AUDIO_RATE) == 44100) ? 1 : 0);
    menu_stat(CONF_AUDLO, (config_get(CONFIG_AUDIO_RATE) == 22050) ? 1 : 0);
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
    menu_link(CONF_GEOHI, CONF_TEXHI, CONF_REFON, CONF_GEOHI, CONF_GEOLO);
    menu_link(CONF_GEOLO, CONF_TEXLO, CONF_REFOF, CONF_GEOHI, CONF_GEOLO);
    menu_link(CONF_REFON, CONF_GEOHI, CONF_AUDHI, CONF_REFON, CONF_REFOF);
    menu_link(CONF_REFOF, CONF_GEOLO, CONF_AUDLO, CONF_REFON, CONF_REFOF);
    menu_link(CONF_AUDHI, CONF_REFON, CONF_SNDDN, CONF_AUDHI, CONF_AUDLO);
    menu_link(CONF_AUDLO, CONF_REFOF, CONF_SNDUP, CONF_AUDHI, CONF_AUDLO);
    menu_link(CONF_SNDDN, CONF_AUDHI, CONF_MUSDN, CONF_SNDDN, CONF_SNDUP);
    menu_link(CONF_SNDUP, CONF_AUDLO, CONF_MUSUP, CONF_SNDDN, CONF_SNDUP);
    menu_link(CONF_MUSDN, CONF_SNDDN, CONF_BACK,  CONF_MUSDN, CONF_MUSUP);
    menu_link(CONF_MUSUP, CONF_SNDUP, CONF_BACK,  CONF_MUSDN, CONF_MUSUP);
    menu_link(CONF_BACK,  CONF_MUSDN, CONF_BACK,  CONF_BACK,  CONF_BACK);
}

static void conf_leave(void)
{
    menu_free();
    back_free();
}

static void conf_paint(float dy)
{
    config_push_persp(FOV, 0.1, FAR_DIST);
    {
        back_draw(0, time_state());
    }
    config_pop_matrix();

    menu_paint();
}

static void conf_point(int x, int y, int dx, int dy)
{
    menu_point(x, y);
}

static void conf_stick(int a, int v)
{
    menu_stick(a, v);
}

static int conf_click(int b, int d)
{
    return (b < 0 && d == 1) ? conf_action(menu_click()) : 1;
}

static int conf_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int conf_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return conf_action(menu_buttn());
        if (config_tst(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_title);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_title);
    }
    return 1;
}
#endif

/*---------------------------------------------------------------------------*/

static int set_value = 0;

static GLuint shot_rect = 0;
static int    shot_set  = 0;

static int set_action(int i)
{
    set_value = i;

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

    char buf[MAXSTR];
    const char *p = set_desc(i);

    text_size("0", TXT_SML, &w, &h);
    y = -2 * h;
    h =  3 * h / 2;

    menu_item(7, 0, -10 * h / 3, h * 15, 5 * h);
    menu_stat(7, -1);

    while (p && m < 12)
    {
        memset(buf, 0, 256);
        j = 0;

        while (p && *p && *p != '\\')
            buf[j++] = *p++;

        menu_text(m, 0, y, c_ylw, c_white, buf, TXT_SML);

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
    menu_init(12, 8, set_value);
    back_init("png/blues.png", config_get(CONFIG_GEOMETRY));

    /* Text elements */

    menu_text(0, -8 * h, 16 * j, c_white,  c_white,      "Back", TXT_SML);
    menu_text(1, +7 * h, 16 * j, c_ylw, c_red,   "Level Set", TXT_SML);
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

    shot_rect = make_rect(config_get(CONFIG_WIDTH) / 2,
                          config_get(CONFIG_HEIGHT) / 2 -      j / 2,
                          config_get(CONFIG_WIDTH) / 2 + 10 * h,
                          config_get(CONFIG_HEIGHT) / 2 + 29 * j / 2);
}

static void set_leave(void)
{
    if (glIsList(shot_rect))
        glDeleteLists(shot_rect, 1);

    menu_free();
}

static void set_paint(float dy)
{
    config_push_persp(FOV, 0.1f, FAR_DIST);
    {
        back_draw(0, time_state());
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

static void set_point(int x, int y, int dx, int dy)
{
    int i;

    if ((i = menu_point(x, y)) >= 0 && shot_set != i - 2)
    {
        shot_set = i - 2;
        set_describe(shot_set);
    }
}

static void set_stick(int a, int v)
{
    menu_stick(a, v);
}

static int set_click(int b, int d)
{
    return (b < 0 && d == 1) ? set_action(menu_click()) : 1;
}

static int set_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int set_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return set_action(menu_buttn());
        if (config_tst(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_title);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_title);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int coin_1c;
static int coin_2c;
static int coin_3c;

static int coin_1n;
static int coin_2n;
static int coin_3n;

static int coin_1t;
static int coin_2t;
static int coin_3t;

/* Build a Most Coins top three list with default values. */

static void gui_most_coins(int id)
{
    const float *c0 = gui_yel;
    const float *c1 = gui_wht;

    int jd, kd, ld, md;

    if ((jd = gui_hstack(id)))
    {
        gui_filler(jd);

        if ((kd = gui_vstack(jd)))
        {
            gui_label(kd, "Most Coins", GUI_SML, GUI_TOP, 0, 0);

            if ((ld = gui_hstack(kd)))
            {
                if ((md = gui_varray(ld)))
                {
                    coin_1c = gui_count(md, 1000,    GUI_SML, 0);
                    coin_2c = gui_count(md, 1000,    GUI_SML, 0);
                    coin_3c = gui_count(md, 1000,    GUI_SML, GUI_SE);
                }
                if ((md = gui_varray(ld)))
                {
                    coin_1n = gui_label(md, "Hard",  GUI_SML, 0, c0, c1);
                    coin_2n = gui_label(md, "Medium",GUI_SML, 0, c0, c1);
                    coin_3n = gui_label(md, "Easy",  GUI_SML, 0, c0, c1);
                }
                if ((md = gui_varray(ld)))
                {
                    coin_1t = gui_clock(md, 60000,   GUI_SML, 0);
                    coin_2t = gui_clock(md, 60000,   GUI_SML, 0);
                    coin_3t = gui_clock(md, 60000,   GUI_SML, GUI_SW);
                }
            }
        }
        gui_filler(jd);
    }
}

/* Set the Most Coins top three list values for level i. */

static void set_most_coins(int i)
{
    gui_set_count(coin_1c, level_coin_c(i, 0), GUI_SML);
    gui_set_count(coin_2c, level_coin_c(i, 1), GUI_SML);
    gui_set_count(coin_3c, level_coin_c(i, 2), GUI_SML);

    gui_set_label(coin_1n, level_coin_n(i, 0), GUI_SML);
    gui_set_label(coin_2n, level_coin_n(i, 1), GUI_SML);
    gui_set_label(coin_3n, level_coin_n(i, 2), GUI_SML);

    gui_set_clock(coin_1t, level_coin_t(i, 0), GUI_SML);
    gui_set_clock(coin_2t, level_coin_t(i, 1), GUI_SML);
    gui_set_clock(coin_3t, level_coin_t(i, 2), GUI_SML);
}

/*---------------------------------------------------------------------------*/

static int time_1c;
static int time_2c;
static int time_3c;

static int time_1n;
static int time_2n;
static int time_3n;

static int time_1t;
static int time_2t;
static int time_3t;

/* Build a Best Times top three list with default values. */

static void gui_best_times(int id)
{
    const float *c0 = gui_yel;
    const float *c1 = gui_wht;

    int jd, kd, ld, md;

    if ((jd = gui_hstack(id)))
    {
        gui_filler(jd);

        if ((kd = gui_vstack(jd)))
        {
            gui_label(kd, "Best Times", GUI_SML, GUI_TOP, 0, 0);

            if ((ld = gui_hstack(kd)))
            {
                if ((md = gui_varray(ld)))
                {
                    time_1t = gui_clock(md, 1000,    GUI_SML, 0);
                    time_2t = gui_clock(md, 1000,    GUI_SML, 0);
                    time_3t = gui_clock(md, 1000,    GUI_SML, GUI_SE);
                }
                if ((md = gui_varray(ld)))
                {
                    time_1n = gui_label(md, "Hard",  GUI_SML, 0, c0, c1);
                    time_2n = gui_label(md, "Medium",GUI_SML, 0, c0, c1);
                    time_3n = gui_label(md, "Easy",  GUI_SML, 0, c0, c1);
                }
                if ((md = gui_varray(ld)))
                {
                    time_1c = gui_count(md, 60000,   GUI_SML, 0);
                    time_1c = gui_count(md, 60000,   GUI_SML, 0);
                    time_1c = gui_count(md, 60000,   GUI_SML, GUI_SW);
                }
            }
        }
        gui_filler(jd);
    }
}

/* Set the Best Times top three list values for level i. */

static void set_best_times(int i)
{
    gui_set_clock(time_1t, level_time_t(i, 0), GUI_SML);
    gui_set_clock(time_2t, level_time_t(i, 1), GUI_SML);
    gui_set_clock(time_3t, level_time_t(i, 2), GUI_SML);

    gui_set_label(time_1n, level_time_n(i, 0), GUI_SML);
    gui_set_label(time_2n, level_time_n(i, 1), GUI_SML);
    gui_set_label(time_3n, level_time_n(i, 2), GUI_SML);

    gui_set_count(time_1c, level_time_c(i, 0), GUI_SML);
    gui_set_count(time_2c, level_time_c(i, 1), GUI_SML);
    gui_set_count(time_3c, level_time_c(i, 2), GUI_SML);
}

/*---------------------------------------------------------------------------*/

/* Create a level selector button based upon its existence and status. */

static void gui_level(int id, char *text, int i)
{
    int o = level_opened(i);
    int e = level_exists(i);

    if      (o) gui_state(id, text, GUI_SML, i, 0);
    else if (e) gui_label(id, text, GUI_SML, GUI_ALL, gui_yel, gui_red);
    else        gui_label(id, text, GUI_SML, GUI_ALL, gui_blk, gui_blk);
}

/*---------------------------------------------------------------------------*/

#define START_BACK 99

static int start_id;

static int start_action(int i)
{
    if (i > 0)
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

static void start_enter(void)
{
    int w = config_get(CONFIG_WIDTH);
    int h = config_get(CONFIG_HEIGHT);

    int id, jd, kd;

    if ((start_id = gui_vstack(0)))
    {
        if ((id = gui_harray(start_id)))
        {
            gui_label(id, "Level", GUI_SML, GUI_ALL, gui_yel, gui_red);
            gui_filler(id);
            gui_filler(id);
            gui_start(id, "Back",  GUI_SML, START_BACK, 0);
        }

        if ((id = gui_harray(start_id)))
        {
            gui_image(id, "shot-rlk/easy.jpg", 3 * w / 8, 3 * h / 8);

            if ((jd = gui_varray(id)))
            {
                if ((kd = gui_harray(jd)))
                {
                    gui_level(kd, "05",  5);
                    gui_level(kd, "04",  4);
                    gui_level(kd, "03",  3);
                    gui_level(kd, "02",  2);
                    gui_level(kd, "01",  1);
                }
                if ((kd = gui_harray(jd)))
                {
                    gui_level(kd, "10", 10);
                    gui_level(kd, "09",  9);
                    gui_level(kd, "08",  8);
                    gui_level(kd, "07",  7);
                    gui_level(kd, "06",  6);
                }
                if ((kd = gui_harray(jd)))
                {
                    gui_level(kd, "15", 15);
                    gui_level(kd, "14", 14);
                    gui_level(kd, "13", 13);
                    gui_level(kd, "12", 12);
                    gui_level(kd, "11", 11);
                }
                if ((kd = gui_harray(jd)))
                {
                    gui_level(kd, "20", 20);
                    gui_level(kd, "19", 19);
                    gui_level(kd, "18", 18);
                    gui_level(kd, "17", 17);
                    gui_level(kd, "16", 16);
                }
                if ((kd = gui_harray(jd)))
                {
                    gui_level(kd, "25", 25);
                    gui_level(kd, "24", 24);
                    gui_level(kd, "23", 23);
                    gui_level(kd, "22", 22);
                    gui_level(kd, "21", 21);
                }
            }
        }
        gui_space(start_id);

        if ((id = gui_harray(start_id)))
        {
            gui_most_coins(id);
            gui_best_times(id);
        }

        gui_layout(start_id, 0, 0);
        set_most_coins(0);
        set_best_times(0);
    }

    back_init("png/blues.png", config_get(CONFIG_GEOMETRY));
}

static void start_leave(void)
{
    gui_delete(start_id);
}

static void start_paint(float dy)
{
    config_push_persp(FOV, 0.1f, FAR_DIST);
    {
        back_draw(0, time_state());
    }
    config_pop_matrix();

    gui_paint(start_id);
}

static void start_timer(float dt)
{
    gui_timer(start_id, dt);
}

static void start_point(int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(start_id, x, y), 1.2f);
}

static void start_stick(int a, int v)
{
    if (config_tst(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(start_id, v, 0), 1.2f);
    if (config_tst(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(start_id, 0, v), 1.2f);
}

static int start_click(int b, int d)
{
    if (d && b < 0)
        return start_action(gui_token(gui_click()));
    return 1;
}

static int start_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        return goto_state(&st_title);

    if (d && c == SDLK_F12)
    {
        int n = curr_count();
        int i;

        /* Iterate over all levels, taking a screenshot of each. */

        back_free();
        {
            for (i = 1; i < n; i++)
                if (level_exists(i))
                    level_snap(i);
        }
        back_init("png/blues.png", config_get(CONFIG_GEOMETRY));
    }

    return 1;
}

static int start_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return start_action(gui_token(gui_click()));
        if (config_tst(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_title);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_title);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void level_enter(void)
{
    int y = 1 * config_get(CONFIG_HEIGHT) / 6;
    int h = 1 * config_get(CONFIG_WIDTH) / 6;
    int w = 7 * config_get(CONFIG_WIDTH) / 8;
    int i, j, l = curr_level();

    char buf[MAXSTR], *p = curr_intro();

    sprintf(buf, "Level %02d", l);
    
    menu_init(7, 2, 10);
    menu_text(0, 0, y, c_ylw, c_red, buf, TXT_LRG);
    menu_item(0, 0, y, w, h);

    /* Position the level intro message text. */

    text_size("M", TXT_SML, NULL, &l);
    y = 0;
    i = 1;

    menu_item(1, 0, -5 * l / 2, w, l * 6);
    menu_stat(1, -1);

    while (p && i < 7)
    {
        memset(buf, 0, MAXSTR);
        j = 0;

        while (p && *p && *p != '\\')
            buf[j++] = *p++;

        if (strlen(buf) > 0)
            menu_text(i, 0, y, c_white, c_white, buf, TXT_SML);

        y -= l;
        i++;

        if (*p) p++;
    }

    game_set_fly(1.f);
}

static void level_leave(void)
{
    menu_free();
}

static void level_paint(float dy)
{
    game_draw(0, dy);
    hud_draw();
    menu_paint();
}

static int level_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_two) : 1;
}

static int level_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        goto_state(&st_over);
    if (d && c == SDLK_F12)
        goto_state(&st_poser);
    return 1;
}

static int level_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_two);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void poser_paint(float dy)
{
    game_draw(1, dy);
}

static int poser_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_level) : 1;
}

/*---------------------------------------------------------------------------*/

static void two_enter(void)
{
    int y = 1 * config_get(CONFIG_HEIGHT) / 6;
    int h = 1 * config_get(CONFIG_WIDTH) / 6;
    int w = 2 * config_get(CONFIG_WIDTH) / 3;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_ylw, c_red, "Ready?", TXT_LRG);
    menu_item(0, 0, y, w, h);

    audio_music_fade(2.f);
    audio_play(AUD_READY, 1.f);
}

static void two_leave(void)
{
    menu_free();
}

static void two_paint(float dy)
{
    game_draw(0, dy);
    hud_draw();
    menu_paint();
}

static void two_timer(float dt)
{
    float t = time_state();

    game_set_fly(1.0f - 0.5f * t);

    if (dt > 0.0f && t > 1.0f)
        goto_state(&st_one);
}

static int two_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_play) : 1;
}

static int two_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_over) : 1;
}

static int two_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_play);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void one_enter(void)
{
    int y = 1 * config_get(CONFIG_HEIGHT) / 6;
    int h = 1 * config_get(CONFIG_WIDTH) / 6;
    int w = 2 * config_get(CONFIG_WIDTH) / 3;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_ylw, c_red, "Set?", TXT_LRG);
    menu_item(0, 0, y, w, h);

    audio_play(AUD_SET, 1.f);
}

static void one_leave(void)
{
    menu_free();
}

static void one_paint(float dy)
{
    game_draw(0, dy);
    hud_draw();
    menu_paint();
}

static void one_timer(float dt)
{
    float t = time_state();

    game_set_fly(0.5f - 0.5f * t);

    if (dt > 0.0f && t > 1.0f)
        goto_state(&st_play);
}

static int one_click(int b, int d)
{
    if (b < 0 && d == 1)
    {
        game_set_fly(0.0f);
        return goto_state(&st_play);
    }
    return 1;
}

static int one_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_over) : 1;
}

static int one_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_play);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int view_rotate = 0;

static void play_enter(void)
{
    char filename[MAXSTR];

    int y = 1 * config_get(CONFIG_HEIGHT) / 6;
    int h = 1 * config_get(CONFIG_WIDTH) / 6;
    int w = 2 * config_get(CONFIG_WIDTH) / 3;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_blue, c_green, "GO!", TXT_LRG);
    menu_item(0, 0, y, w, h);

    level_song();
    audio_play(AUD_GO, 1.f);

    game_set_fly(0.f);
    view_rotate = 0;

    if (config_home(filename, USER_REPLAY_FILE, MAXSTR))
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

static void play_paint(float dy)
{
    game_draw(0, dy);
    hud_draw();

    if (time_state() < 1.f)
        menu_paint();
}

static void play_timer(float dt)
{
    static float at = 0;

    float g[3] = { 0.0f, -9.8f, 0.0f };

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
        float_put(record_fp, &at);
        game_put(record_fp);
    }
}

static void play_point(int x, int y, int dx, int dy)
{
    game_set_pos(dx, dy);
}

static void play_stick(int a, int k)
{
    if (config_tst(CONFIG_JOYSTICK_AXIS_X, a))
        game_set_z(k);
    if (config_tst(CONFIG_JOYSTICK_AXIS_Y, a))
        game_set_x(k);
}

static int play_click(int b, int d)
{
    view_rotate = d ? b : 0;
    return 1;
}

static int play_keybd(int c, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_KEY_CAMERA_R, c))
            view_rotate = +1;
        if (config_tst(CONFIG_KEY_CAMERA_L, c))
            view_rotate = -1;

        if (config_tst(CONFIG_KEY_CAMERA_1, c))
        {
            config_set(CONFIG_CAMERA, 0);
            hud_view_pulse(0);
        }
        if (config_tst(CONFIG_KEY_CAMERA_2, c))
        {
            config_set(CONFIG_CAMERA, 1);
            hud_view_pulse(1);
        }
        if (config_tst(CONFIG_KEY_CAMERA_3, c))
        {
            config_set(CONFIG_CAMERA, 2);
            hud_view_pulse(2);
        }
    }
    else
    {
        if (config_tst(CONFIG_KEY_CAMERA_R, c))
            view_rotate = 0;
        if (config_tst(CONFIG_KEY_CAMERA_L, c))
            view_rotate = 0;
    }

    if (d && c == SDLK_ESCAPE)
        goto_state(&st_over);
    return 1;
}

static int play_buttn(int b, int d)
{
    if (d == 1)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);

        if (config_tst(CONFIG_JOYSTICK_BUTTON_R, b))
            view_rotate = +1;
        if (config_tst(CONFIG_JOYSTICK_BUTTON_L, b))
            view_rotate = -1;
    }
    else
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_R, b))
            view_rotate = 0;
        if (config_tst(CONFIG_JOYSTICK_BUTTON_L, b))
            view_rotate = 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void demo_enter(void)
{
    char filename[MAXSTR];

    int y = 1 * config_get(CONFIG_HEIGHT) / 6;
    int h = 1 * config_get(CONFIG_WIDTH) / 6;
    int w = 2 * config_get(CONFIG_WIDTH) / 3;

    menu_init(1, 1, 1);
    menu_text(0, 0, y, c_blue, c_green, "Demo", TXT_LRG);
    menu_item(0, 0, y, w, h);

    global_time = -1.f;
    replay_time =  0.f;

    if (config_home(filename, USER_REPLAY_FILE, MAXSTR))
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

    game_set_fly(0.f);
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

static void demo_paint(float dy)
{
    game_draw(0, dy);
    hud_draw();

    if (time_state() < 1.f)
        menu_paint();
}

static void demo_timer(float dt)
{
    float g[3] = { 0.0f, -9.8f, 0.0f };
    float t;
    int    b = 1;

    global_time += dt;

    if (replay_fp)
    {
        /* Spin or skip depending on how fast the demo wants to run. */

        while (replay_time < global_time && (b = float_get(replay_fp, &t)))
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
}

static int demo_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        goto_state(&st_omed);
    return 1;
}

static int demo_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_omed);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void goal_enter(void)
{
    /*
    int y = 1 * config_get(CONFIG_HEIGHT) / 6;
    int h = 1 * config_get(CONFIG_WIDTH) / 6;
    int w = 2 * config_get(CONFIG_WIDTH) / 3;

    menu_init(25, 5, 99);
    menu_text(0, 0, y, c_blue, c_green, "GOAL!", TXT_LRG);
    menu_item(0, 0, y, w, h);

    score_menu(curr_level(), 1, 1);
    highs_menu(curr_level(), 5, 3);
    */
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

static void goal_paint(float dy)
{
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

static int goal_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        goto_state(&st_over);
    return 1;
}

static void goal_timer(float dt)
{
    float g[3] = { 0.0f, 9.8f, 0.0f };
    float t = dt;

    if (time_state() < 1.f)
    {
        game_step(g, dt, 0);

        if (record_fp)
        {
            float_put(record_fp, &t);
            game_put(record_fp);
        }
    }
}

static int goal_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return goal_click(0, 1);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int score_value = 28;

static int score_action(int i)
{
    char player[MAXNAM];
    int w, h;
	size_t l;

    config_get_name(player);
    l = strlen(player);

    score_value = i;

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

        config_set_name(player);
    }
    else
    {
        /* Letter selected */

        if (l < MAXNAM - 1)
        {
            player[l + 0] = i + 'A' - 1;
            player[l + 1] = 0;
        }

        config_set_name(player);
    }

    text_size("M", TXT_MED, &w, &h);
    menu_text(0, 0, h & (~3), c_ylw, c_white, player, TXT_MED);

    return 1;
}

static void score_enter(void)
{
    char player[MAXNAM];
    int i;
    int sw, mw, lw;
    int sh, mh, lh;

    config_get_name(player);

    text_size("M", TXT_SML, &sw, &sh);
    text_size("M", TXT_MED, &mw, &mh);
    text_size("M", TXT_LRG, &lw, &lh);
    sw *= 2;
    sh *= 2;

    menu_init(34, 32, score_value);

    /* Text elements */

    menu_text(0,  0,     mh, c_ylw, c_white, player,        TXT_MED);
    menu_text(29, 0, 3 * mh, c_ylw, c_red,   "New Record!", TXT_LRG);

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

    /*
    score_menu(curr_level(), 30, 30);
    */
    /* Active elements. */

    menu_item(0,  0,     mh, mw * MAXNAM,        mh);
    menu_item(29, 0, 3 * mh, 8 * config_get(CONFIG_WIDTH) / 9, lh);

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
}

static void score_leave(void)
{
    menu_free();
}

static void score_paint(float dy)
{
    game_draw(0, dy);
    menu_paint();
}

static void score_point(int x, int y, int dx, int dy)
{
    menu_point(x, y);
}

static void score_stick(int a, int v)
{
    menu_stick(a, v);
}

static int score_click(int b, int d)
{
    return (b < 0 && d == 1) ? score_action(menu_click()) : 1;
}

static int score_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        goto_state(&st_over);
    return 1;
}

static int score_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return score_action(menu_buttn());
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void fall_enter(void)
{
    int y = 1 * config_get(CONFIG_HEIGHT) / 6;
    int h = 1 * config_get(CONFIG_WIDTH) / 6;
    int w = 7 * config_get(CONFIG_WIDTH) / 8;

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

static void fall_paint(float dy)
{
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

static int fall_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        goto_state(&st_over);
    return 1;
}

static void fall_timer(float dt)
{
    float g[3] = { 0.0f, -9.8f, 0.0f };
    float t = dt;

    if (time_state() < 2.f)
    {
        game_step(g, dt, 0);

        if (record_fp)
        {
            float_put(record_fp, &t);
            game_put(record_fp);
        }
    }
    else
        fall_click(0, 1);
}

static int fall_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return fall_click(0, 1);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void time_enter(void)
{
    int y = 1 * config_get(CONFIG_HEIGHT) / 6;
    int h = 1 * config_get(CONFIG_WIDTH) / 6;
    int w = 7 * config_get(CONFIG_WIDTH) / 8;

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

static void time_paint(float dy)
{
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

static void time_timer(float dt)
{
    float g[3] = { 0.0f, -9.8f, 0.0f };
    float t = dt;

    if (time_state() < 2.f)
    {
        game_step(g, dt, 0);

        if (record_fp)
        {
            float_put(record_fp, &t);
            game_put(record_fp);
        }
    }
    else
        time_click(0, 1);
}

static int time_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return time_click(0, 1);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void omed_enter(void)
{
    int y = 1 * config_get(CONFIG_HEIGHT) / 6;
    int h = 1 * config_get(CONFIG_WIDTH) / 6;
    int w = 7 * config_get(CONFIG_WIDTH) / 8;

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

static void omed_paint(float dy)
{
    game_draw(0, dy);
    menu_paint();
}

static void omed_timer(float dt)
{
    if (dt > 0.f && time_state() > 3.f)
        goto_state(&st_title);
}

static int omed_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int omed_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_title) : 1;
}

static int omed_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_title);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_title);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_title);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void over_enter(void)
{
    int y = 1 * config_get(CONFIG_HEIGHT) / 6;
    int h = 1 * config_get(CONFIG_WIDTH) / 6;
    int w = 7 * config_get(CONFIG_WIDTH) / 8;

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

static void over_paint(float dy)
{
    game_draw(0, dy);
    menu_paint();
}

static void over_timer(float dt)
{
    if (dt > 0.f && time_state() > 3.f)
        goto_state(&st_title);
}

static int over_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int over_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_title) : 1;
}

static int over_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_title);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_title);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_title);
    }
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
    menu_text(0, 0, sh + lm, c_green, c_ylw, STR_DONE, TXT_LRG);
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

static void done_paint(float dy)
{
    game_draw(0, dy);
    menu_paint();
}

static void done_timer(float dt)
{
    if (dt > 0.f && time_state() > 10.f)
        goto_state(&st_title);
}

static int done_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int done_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_title) : 1;
}

static int done_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_title);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_title);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_title);
    }
    return 1;
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
    NULL,
    NULL,
    1
};

struct state st_title = {
    title_enter,
    title_leave,
    title_paint,
    title_timer,
    title_point,
    title_stick,
    title_click,
    title_keybd,
    title_buttn,
    1
};

struct state st_help = {
    help_enter,
    help_leave,
    help_paint,
    NULL,
    NULL,
    NULL,
    help_click,
    help_keybd,
    help_buttn,
    1
};

struct state st_set = {
    set_enter,
    set_leave,
    set_paint,
    NULL,
    set_point,
    set_stick,
    set_click,
    set_keybd,
    set_buttn,
    1
};

struct state st_start = {
    start_enter,
    start_leave,
    start_paint,
    start_timer,
    start_point,
    start_stick,
    start_click,
    start_keybd,
    start_buttn,
    1
};

struct state st_conf = {
    conf_enter,
    conf_leave,
    conf_paint,
    conf_timer,
    conf_point,
    conf_stick,
    conf_click,
    conf_keybd,
    conf_buttn,
    1
};

struct state st_level = {
    level_enter,
    level_leave,
    level_paint,
    NULL,
    NULL,
    NULL,
    level_click,
    level_keybd,
    level_buttn,
    1
};

struct state st_poser = {
    NULL,
    NULL,
    poser_paint,
    NULL,
    NULL,
    NULL,
    NULL,
    poser_keybd,
    NULL,
    1
};

struct state st_two = {
    two_enter,
    two_leave,
    two_paint,
    two_timer,
    NULL,
    NULL,
    two_click,
    two_keybd,
    two_buttn,
    1
};

struct state st_one = {
    one_enter,
    one_leave,
    one_paint,
    one_timer,
    NULL,
    NULL,
    one_click,
    one_keybd,
    one_buttn,
    1
};

struct state st_play = {
    play_enter,
    play_leave,
    play_paint,
    play_timer,
    play_point,
    play_stick,
    play_click,
    play_keybd,
    play_buttn,
    0
};

struct state st_demo = {
    demo_enter,
    demo_leave,
    demo_paint,
    demo_timer,
    NULL,
    NULL,
    NULL,
    demo_keybd,
    demo_buttn,
    0
};

struct state st_goal = {
    goal_enter,
    goal_leave,
    goal_paint,
    goal_timer,
    NULL,
    NULL,
    goal_click,
    goal_keybd,
    goal_buttn,
    1
};

struct state st_score = {
    score_enter,
    score_leave,
    score_paint,
    NULL,
    score_point,
    score_stick,
    score_click,
    score_keybd,
    score_buttn,
    1
};

struct state st_fall = {
    fall_enter,
    fall_leave,
    fall_paint,
    fall_timer,
    NULL,
    NULL,
    fall_click,
    fall_keybd,
    fall_buttn,
    1
};

struct state st_time = {
    time_enter,
    time_leave,
    time_paint,
    time_timer,
    NULL,
    NULL,
    time_click,
    NULL,
    time_buttn,
    1
};

struct state st_omed = {
    omed_enter,
    omed_leave,
    omed_paint,
    omed_timer,
    NULL,
    NULL,
    omed_click,
    omed_keybd,
    omed_buttn,
    1
};

struct state st_over = {
    over_enter,
    over_leave,
    over_paint,
    over_timer,
    NULL,
    NULL,
    over_click,
    over_keybd,
    over_buttn,
    1
};

struct state st_done = {
    done_enter,
    done_leave,
    done_paint,
    done_timer,
    NULL,
    NULL,
    done_click,
    done_keybd,
    done_buttn,
    1
};

