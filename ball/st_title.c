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
#include "vec3.h"
#include "back.h"
#include "demo.h"
#include "game.h"
#include "audio.h"
#include "config.h"

#include "st_title.h"
#include "st_demo.h"
#include "st_conf.h"
#include "st_set.h"

/*---------------------------------------------------------------------------*/

static float real_time = 0.0f;
static float demo_time = 0.0f;
static int   mode      = 0;

#define TITLE_PLAY 1
#define TITLE_HELP 2
#define TITLE_DEMO 3
#define TITLE_CONF 4
#define TITLE_EXIT 5

static int title_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case TITLE_PLAY: return goto_state(&st_set);
    case TITLE_HELP: return goto_state(&st_help);
    case TITLE_DEMO: return goto_state(&st_demo);
    case TITLE_CONF: return goto_state(&st_conf);
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
        gui_label(id, _("Neverball"), GUI_LRG, GUI_ALL, 0, 0);
        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_filler(jd);

            if ((kd = gui_varray(jd)))
            {
                gui_start(kd, sgettext("menu^Play"),    GUI_MED, TITLE_PLAY, 1);
                gui_state(kd, sgettext("menu^Replay"),  GUI_MED, TITLE_DEMO, 0);
                gui_state(kd, sgettext("menu^Help"),    GUI_MED, TITLE_HELP, 0);
                gui_state(kd, sgettext("menu^Options"), GUI_MED, TITLE_CONF, 0);
                gui_state(kd, sgettext("menu^Exit"),    GUI_MED, TITLE_EXIT, 0);
            }

            gui_filler(jd);
        }
        gui_layout(id, 0, 0);
    }

    /* Start the title screen music. */

    audio_music_fade_to(0.5f, "bgm/title.ogg");

    /* Initialize the first level of the first set for display. */

    game_init("map-rlk/title.sol",
              "map-back/jupiter.sol", "png/space.png", 0, 1);

    real_time = 0.0f;
    demo_time = 0.0f;
    mode = 0;

    return id;
}

static void title_leave(int id)
{
    demo_replay_stop(0);
    gui_delete(id);
}

static void title_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void title_timer(int id, float dt)
{
    static const char *demo = NULL;
    float t;

    real_time += dt;

    switch (mode)
    {
    case 0: /* Mode 0: Pan across title level. */

        if (real_time <= 20.0f)
            game_set_fly(fcosf(V_PI * real_time / 20.0f));
        else
        {
            game_fade(+1.0f);
            real_time = 0.0f;
            mode = 1;
        }
        break;

    case 1: /* Mode 1: Fade out.  Load demo level. */

        if (real_time > 1.0f)
        {
            if ((demo = demo_pick()))
            {
                demo_replay_init(demo, NULL, NULL, NULL, NULL, NULL);
                demo_time = 0.0f;
                real_time = 0.0f;
                mode = 2;
            }
            else
            {
                game_fade(-1.0f);
                real_time = 0.0f;
                mode = 0;
            }
        }
        break;

    case 2: /* Mode 2: Run demo. */

        while (demo_time < real_time)
            if (demo_replay_step(&t))
                demo_time += t;
            else
            { 
                demo_replay_stop(0);
                game_fade(+1.0f);
                real_time = 0.0f;
                mode = 3;
            }
        break;

    case 3: /* Mode 3: Fade out.  Load title level. */

        if (real_time > 1.0f)
        {
            game_init("map-rlk/title.sol",
                      "map-back/jupiter.sol", "png/space.png", 0, 1);
            real_time = 0.0f;
            mode = 0;
        }
        break;
    }

    gui_timer(id, dt);
    audio_timer(dt);
    game_step_fade(dt);
}

static void title_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void title_stick(int id, int a, int v)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int title_click(int b, int d)
{
    if (d && b < 0)
        return title_action(gui_token(gui_click()));
    return 1;
}

static int title_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return title_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int help_enter(void)
{
    const char *s0 =
        _("Move the mouse or joystick to tilt the floor\\"
        "causing the ball to roll.  Roll over coins to\\"
        "collect them.  Collect coins to unlock the goal\\"
        "and finish the level.  Earn an extra ball for\\"
        "each 100 coins collected.\\");

    const char *s4 = _("Left and right mouse buttons rotate the view.");
    const char *s5 = _("Hold Shift for faster view rotation.");
    const char *s6 = _("Pause / Release Pointer");
    const char *s7 = _("Exit / Cancel Menu");
    const char *s8 = _("Chase View");
    const char *s9 = _("Lazy View");
    const char *sA = _("Manual View");
    const char *sB = _("Comments?  Problems?  robert.kooima@gmail.com");

    const char *k0 = _("Spacebar");
    const char *k1 = _("Escape");
    const char *k2 = SDL_GetKeyName(config_get_d(CONFIG_KEY_CAMERA_1));
    const char *k3 = SDL_GetKeyName(config_get_d(CONFIG_KEY_CAMERA_2));
    const char *k4 = SDL_GetKeyName(config_get_d(CONFIG_KEY_CAMERA_3));

    int id, jd;

    if ((id = gui_vstack(0)))
    {
        gui_multi(id, s0, GUI_SML, GUI_ALL, gui_wht, gui_wht);
        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_label(jd, s6, GUI_SML, GUI_NE, gui_wht, gui_wht);
            gui_label(jd, k0, GUI_SML, GUI_NW, gui_yel, gui_yel);
        }
        if ((jd = gui_harray(id)))
        {
            gui_label(jd, s7, GUI_SML, 0,      gui_wht, gui_wht);
            gui_label(jd, k1, GUI_SML, 0,      gui_yel, gui_yel);
        }
        if ((jd = gui_harray(id)))
        {
            gui_label(jd, s8, GUI_SML, 0,      gui_wht, gui_wht);
            gui_label(jd, k2, GUI_SML, 0,      gui_yel, gui_yel);
        }
        if ((jd = gui_harray(id)))
        {
            gui_label(jd, s9, GUI_SML, 0,      gui_wht, gui_wht);
            gui_label(jd, k3, GUI_SML, 0,      gui_yel, gui_yel);
        }
        if ((jd = gui_harray(id)))
        {
            gui_label(jd, sA, GUI_SML, GUI_SE, gui_wht, gui_wht);
            gui_label(jd, k4, GUI_SML, GUI_SW, gui_yel, gui_yel);
        }

        gui_space(id);
        gui_label(id, s4, GUI_SML, GUI_TOP, gui_wht, gui_wht);
        gui_label(id, s5, GUI_SML, GUI_BOT, gui_wht, gui_wht);
        gui_space(id);
        gui_label(id, sB, GUI_SML, GUI_ALL, gui_wht, gui_wht);

        gui_layout(id, 0, 0);
    }
    return id;
}

static void help_leave(int id)
{
    gui_delete(id);
}

static void help_paint(int id, float st)
{
    game_draw(0, st);
    config_pop_matrix();
    gui_paint(id);
}

static void help_timer(int id, float dt)
{
    gui_timer(id, dt);
    audio_timer(dt);
}

static int help_click(int b, int d)
{
    return d ? goto_state(&st_title) : 1;
}

static int help_keybd(int c, int d)
{
    return d ? goto_state(&st_title) : 1;
}

static int help_buttn(int b, int d)
{
    return d ? goto_state(&st_title) : 1;
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
    NULL,
    title_buttn,
    1, 0
};

struct state st_help = {
    help_enter,
    help_leave,
    help_paint,
    help_timer,
    NULL,
    NULL,
    help_click,
    help_keybd,
    help_buttn,
    1, 0
};
