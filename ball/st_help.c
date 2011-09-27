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
#include "audio.h"
#include "config.h"
#include "demo.h"
#include "keynames.h"

#include "game_common.h"
#include "game_server.h"
#include "game_client.h"

#include "st_title.h"
#include "st_help.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

enum
{
    HELP_BACK = GUI_LAST,
    HELP_PAGE,
    HELP_DEMO
};

enum
{
    PAGE_RULES,
    PAGE_CONTROLS,
    PAGE_MODES,
    PAGE_TRICKS
};

static const char demos[][16] = {
    "gui/demo1.nbr",
    "gui/demo2.nbr"
};

static int page = PAGE_RULES;

/*---------------------------------------------------------------------------*/

static int help_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case HELP_BACK:
        page = PAGE_RULES;
        return goto_state(&st_title);

    case HELP_DEMO:
        if (demo_replay_init(demos[val], NULL, NULL, NULL, NULL, NULL))
            return goto_state(&st_help_demo);
        break;

    case HELP_PAGE:
        page = val;
        return goto_state(&st_help);

    }
    return 1;
}

/* -------------------------------------------------------------------------- */

static int help_button(int id, const char *text, int token, int value)
{
    int jd = gui_state(id, text, GUI_SML, token, value);

    /* Hilight current page. */

    if (token == HELP_PAGE && value == page)
    {
        gui_set_hilite(jd, 1);
        gui_focus(jd);
    }

    return jd;
}

static int help_menu(int id)
{
    int jd;

    gui_space(id);

    if ((jd = gui_harray(id)))
    {
        help_button(jd, _("Tricks"),   HELP_PAGE, PAGE_TRICKS);
        help_button(jd, _("Modes"),    HELP_PAGE, PAGE_MODES);
        help_button(jd, _("Controls"), HELP_PAGE, PAGE_CONTROLS);
        help_button(jd, _("Rules"),    HELP_PAGE, PAGE_RULES);
        help_button(jd, _("Back"),     HELP_BACK, 0);
    }
    return jd;
}

/* -------------------------------------------------------------------------- */

static int page_rules(int id)
{
    const char *s0 = _(
        "Move the mouse or joystick\\"
        "or use keyboard arrows to\\"
        "tilt the floor causing the\\"
        "ball to roll.\\");
    const char *s1 = _(
        "Roll over coins to collect\\"
        "them.  Collect coins to\\"
        "unlock the goal and finish\\"
        "the level.\\");

    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);

    int jd, kd, ld;

    if ((jd = gui_hstack(id)))
    {
        gui_filler(jd);

        if ((kd = gui_varray(jd)))
        {
            if ((ld = gui_vstack(kd)))
            {
                gui_space(ld);
                gui_multi(ld, s0, GUI_SML, GUI_ALL, gui_wht, gui_wht);
                gui_filler(ld);
            }

            if ((ld = gui_vstack(kd)))
            {
                gui_space(ld);
                gui_multi(ld, s1, GUI_SML, GUI_ALL, gui_wht, gui_wht);
                gui_filler(ld);
            }
        }

        gui_space(jd);

        if ((kd = gui_varray(jd)))
        {
            if ((ld = gui_vstack(kd)))
            {
                gui_space(ld);
                gui_image(ld, "gui/help1.jpg", 5 * w / 16, 5 * h / 16);
                gui_filler(ld);
            }

            if ((ld = gui_vstack(kd)))
            {
                gui_space(ld);
                gui_image(ld, "gui/help2.jpg", 5 * w / 16, 5 * h / 16);
                gui_filler(ld);
            }
        }

        gui_filler(jd);
    }
    return id;
}

static int page_controls(int id)
{
    const char *s4 = _("Left and right mouse buttons rotate the view.");
    const char *s5 = _("Hold Shift for faster view rotation.");
    const char *s6 = _("Pause / Release Pointer");
    const char *s7 = _("Exit / Cancel Menu");
    const char *s8 = _("Chase View");
    const char *s9 = _("Lazy View");
    const char *sA = _("Manual View");
    const char *sC = _("Screenshot");

    const char *k0 = pretty_keyname((SDLKey) config_get_d(CONFIG_KEY_PAUSE));
    const char *k1 = pretty_keyname(SDLK_ESCAPE);
    const char *k2 = pretty_keyname((SDLKey) config_get_d(CONFIG_KEY_CAMERA_1));
    const char *k3 = pretty_keyname((SDLKey) config_get_d(CONFIG_KEY_CAMERA_2));
    const char *k4 = pretty_keyname((SDLKey) config_get_d(CONFIG_KEY_CAMERA_3));
    const char *k6 = pretty_keyname(SDLK_F10);

    int jd;

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
        gui_label(jd, sA, GUI_SML, 0,      gui_wht, gui_wht);
        gui_label(jd, k4, GUI_SML, 0,      gui_yel, gui_yel);
    }
    if ((jd = gui_harray(id)))
    {
        gui_label(jd, sC, GUI_SML, GUI_SE, gui_wht, gui_wht);
        gui_label(jd, k6, GUI_SML, GUI_SW, gui_yel, gui_yel);
    }

    gui_space(id);

    gui_label(id, s4, GUI_SML, GUI_TOP, gui_wht, gui_wht);
    gui_label(id, s5, GUI_SML, GUI_BOT, gui_wht, gui_wht);

    return id;
}

static int page_modes(int id)
{
    int jd, kd;

    gui_space(id);

    if ((jd = gui_hstack(id)))
    {
        gui_filler(jd);

        if ((kd = gui_vstack(jd)))
        {
            gui_label(kd, _("Normal Mode"), GUI_SML, GUI_TOP, 0, 0);
            gui_multi(kd,
                      _("Finish a level before the time runs out.\\"
                        "You need to collect coins in order to open the goal."),
                      GUI_SML, GUI_BOT, gui_wht, gui_wht);

            gui_space(kd);

            gui_label(kd, _("Challenge Mode"), GUI_SML, GUI_TOP, 0, 0);
            gui_multi(kd,
                      _("Start playing from the first level of the set.\\"
                        "You start with only three balls, do not lose them.\\"
                        "Earn an extra ball for each 100 coins collected."),
                      GUI_SML, GUI_BOT, gui_wht, gui_wht);
        }

        gui_filler(jd);
    }
    return id;
}

static int page_tricks(int id)
{
    const char *s0 = _(
        "Corners can be used to jump.\\"
        "Get rolling and take aim\\"
        "at the angle. You may be able\\"
        "to reach new places.\\");
    const char *s1 = _(
        "Tilting in 2 directions increases\\"
        "the slope. Use the manual view\\"
        "and turn the camera by 45\\"
        "degrees for best results.\\");

    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);

    int jd, kd, ld;

    if ((jd = gui_hstack(id)))
    {
        gui_filler(jd);

        if ((kd = gui_varray(jd)))
        {
            if ((ld = gui_vstack(kd)))
            {
                gui_space(ld);
                gui_image(ld, "gui/help3.jpg", w / 4, h / 4);
                gui_state(ld, _("Watch demo"), GUI_SML, 0, 0);
                gui_filler(ld);

                gui_set_state(ld, HELP_DEMO, 0);
            }

            if ((ld = gui_vstack(kd)))
            {
                gui_space(ld);
                gui_image(ld, "gui/help4.jpg", w / 4, h / 4);
                gui_state(ld, _("Watch demo"), GUI_SML, 0, 0);
                gui_filler(ld);

                gui_set_state(ld, HELP_DEMO, 1);
            }
        }

        gui_space(jd);

        if ((kd = gui_varray(jd)))
        {
            if ((ld = gui_vstack(kd)))
            {
                gui_space(ld);
                gui_multi(ld, s0, GUI_SML, GUI_ALL, gui_wht, gui_wht);
                gui_filler(ld);
            }

            if ((ld = gui_vstack(kd)))
            {
                gui_space(ld);
                gui_multi(ld, s1, GUI_SML, GUI_ALL, gui_wht, gui_wht);
                gui_filler(ld);
            }
        }

        gui_filler(jd);
    }
    return id;
}

/* -------------------------------------------------------------------------- */

static int help_gui(void)
{
    int id;

    if ((id = gui_vstack(0)))
    {
        help_menu(id);

        switch (page)
        {
        case PAGE_RULES:    page_rules(id);    break;
        case PAGE_CONTROLS: page_controls(id); break;
        case PAGE_MODES:    page_modes(id);    break;
        case PAGE_TRICKS:   page_tricks(id);   break;
        }

        gui_layout(id, 0, +1);
    }

    return id;
}

static int help_enter(struct state *st, struct state *prev)
{
    return help_gui();
}

static int help_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return help_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return help_action(HELP_BACK, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int help_demo_enter(struct state *st, struct state *prev)
{
    game_client_fly(0.0f);
    return 0;
}

static void help_demo_leave(struct state *st, struct state *next, int id)
{
    demo_replay_stop(0);
}

static void help_demo_paint(int id, float t)
{
    game_client_draw(0, t);
}

static void help_demo_timer(int id, float dt)
{
    game_step_fade(dt);

    if (!demo_replay_step(dt))
        goto_state(&st_help);

    game_client_blend(demo_replay_blend());
}

static int help_demo_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_help);
    }
    return 1;
}
/*---------------------------------------------------------------------------*/

struct state st_help = {
    help_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    NULL,
    help_buttn
};

struct state st_help_demo = {
    help_demo_enter,
    help_demo_leave,
    help_demo_paint,
    help_demo_timer,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    help_demo_buttn
};
