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

#include "gui.h"
#include "set.h"
#include "vec3.h"
#include "back.h"
#include "demo.h"
#include "game.h"
#include "level.h"
#include "audio.h"
#include "config.h"

#include "st_title.h"
#include "st_demo.h"
#include "st_conf.h"
#include "st_set.h"

/*---------------------------------------------------------------------------*/

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

static int title_enter(void)
{
    int id, jd, kd;

    /* Build the title GUI. */

    if ((id = gui_vstack(0)))
    {
        gui_label(id, "Neverball", GUI_LRG, GUI_ALL, 0, 0);
        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_filler(jd);

            if ((kd = gui_varray(jd)))
            {
                gui_start(kd, "Play",     GUI_MED, TITLE_PLAY, 1);
                gui_state(kd, "Help",     GUI_MED, TITLE_HELP, 0);

                if (demo_exists())
                    gui_state(kd, "Demo", GUI_MED, TITLE_DEMO, 0);

                gui_state(kd, "Options",  GUI_MED, TITLE_CONF, 0);
                gui_state(kd, "Exit",     GUI_MED, TITLE_EXIT, 0);
            }

            gui_filler(jd);
        }
        gui_layout(id, 0, 0);
    }

    /* Start the title screen music. */

    audio_music_play("bgm/title.ogg");

    /* Initialize the first level of the first set for display. */

    set_init();
    set_goto(0);
    level_goto(0, 0, 0, 0);

    return id;
}

static void title_leave(int id)
{
    audio_music_play("bgm/inter.ogg");
    gui_delete(id);
    set_free();
}

static void title_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void title_timer(int id, float dt)
{
    game_set_fly(fcosf(time_state() / 10.f));
    gui_timer(id, dt);
}

static void title_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void title_stick(int id, int a, int v)
{
    if (config_tst(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
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

static int help_enter(void)
{
    const char *s0 =
        "Move the mouse or joystick to tilt the floor \\"
        "causing the ball to roll.  Guide it to the goal \\"
        "to finish the level.  Collect 100 coins to earn \\"
        "an extra ball and save your progress. \\";

    const char *s4 = "Left and right mouse buttons rotate the view.";
    const char *s5 = "Pause / Release Pointer";
    const char *s6 = "Exit / Cancel Menu";
    const char *s7 = "Chase View";
    const char *s8 = "Lazy View";
    const char *s9 = "Manual View";

    const char *k0 = "Spacebar";
    const char *k1 = "Escape";
    const char *k2 = SDL_GetKeyName(config_get(CONFIG_KEY_CAMERA_1));
    const char *k3 = SDL_GetKeyName(config_get(CONFIG_KEY_CAMERA_2));
    const char *k4 = SDL_GetKeyName(config_get(CONFIG_KEY_CAMERA_3));

    int id, jd;

    if ((id = gui_vstack(0)))
    {
        gui_multi(id, s0, GUI_SML, GUI_ALL, gui_wht, gui_wht);
        gui_filler(id);

        if ((jd = gui_harray(id)))
        {
            gui_label(jd, s5, GUI_SML, GUI_NE, gui_wht, gui_wht);
            gui_label(jd, k0, GUI_SML, GUI_NW, gui_yel, gui_yel);
        }
        if ((jd = gui_harray(id)))
        {
            gui_label(jd, s6, GUI_SML, 0,      gui_wht, gui_wht);
            gui_label(jd, k1, GUI_SML, 0,      gui_yel, gui_yel);
        }
        if ((jd = gui_harray(id)))
        {
            gui_label(jd, s7, GUI_SML, 0,      gui_wht, gui_wht);
            gui_label(jd, k2, GUI_SML, 0,      gui_yel, gui_yel);
        }
        if ((jd = gui_harray(id)))
        {
            gui_label(jd, s8, GUI_SML, 0,      gui_wht, gui_wht);
            gui_label(jd, k3, GUI_SML, 0,      gui_yel, gui_yel);
        }
        if ((jd = gui_harray(id)))
        {
            gui_label(jd, s9, GUI_SML, GUI_SE, gui_wht, gui_wht);
            gui_label(jd, k4, GUI_SML, GUI_SW, gui_yel, gui_yel);
        }

        gui_filler(id);
        gui_label(id, s4, GUI_SML, GUI_ALL, gui_wht, gui_wht);

        gui_layout(id, 0, 0);
    }

    back_init("png/blues.png", config_get(CONFIG_GEOMETRY));

    return id;
}

static void help_leave(int id)
{
    back_free();
    gui_delete(id);
}

static void help_paint(int id, float st)
{
    config_push_persp((float) config_get(CONFIG_VIEW_FOV), 0.1f, FAR_DIST);
    {
        back_draw(time_state());
    }
    config_pop_matrix();
    gui_paint(id);
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
    1, 0
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
    1, 0
};
