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
#include "game.h"
#include "audio.h"
#include "config.h"
#include "st_shared.h"

#include "st_title.h"
#include "st_help.h"

/*---------------------------------------------------------------------------*/

#define HELP_BACK   1
#define HELP_RULE   2
#define HELP_CONT   3
#define HELP_MODE   4
#define HELP_TECH   5

struct state st_help_rules;
struct state st_help_controls;
struct state st_help_modes;
struct state st_help_techniques;

static int help_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
        case HELP_BACK: return goto_state(&st_title);
        case HELP_RULE: return goto_state(&st_help_rules);
        case HELP_CONT: return goto_state(&st_help_controls);
        case HELP_MODE: return goto_state(&st_help_modes);
        case HELP_TECH: return goto_state(&st_help_techniques);
    }
    return 1;
}

static int help_button(int id, const char *text, int token, int atoken)
{
    int kd = gui_state(id, text, GUI_SML, token, token == atoken);

    if (token == atoken)
        gui_focus(kd);

    return kd;
}

static int help_menu(int id, int i)
{
    int jd;

    gui_filler(id);
    if ((jd = gui_harray(id)))
    {
        help_button(jd, _("Techniques"),  HELP_TECH, i);
        help_button(jd, _("Modes"),    HELP_MODE, i);
        help_button(jd, _("Controls"), HELP_CONT, i);
        help_button(jd, _("Rules"),    HELP_RULE, i);
        help_button(jd, _("Back"),     HELP_BACK, i);
    }
    gui_filler(id);
    return jd;
}

static int help_enter(void)
{
    goto_state(&st_help_rules);
    return 0;
}

static int help_rules_enter(void)
{
    const char *s0 = _(
            "Move the mouse or joystick\\"
            "or use keyboard arrows to\\"
            "tilt the floor causing the\\"
            "ball to roll.\\ ");
    const char *s1 = _(
            "Roll over coins to collect\\"
            "them.  Collect coins to\\"
            "unlock the goal and finish\\"
            "the level.\\ ");

    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);

    int id, jd;

    if ((id = gui_vstack(0)))
    {
        help_menu(id, HELP_RULE);

        if ((jd = gui_hstack(id)))
        {
            gui_image(jd, "gui/help1.jpg", 6 * w / 16, 6 * h / 16);
            gui_multi(jd, s0, GUI_SML, GUI_ALL, gui_wht, gui_wht);
            gui_filler(jd);
        }
        gui_space(id);
        if ((jd = gui_hstack(id)))
        {
            gui_filler(jd);
            gui_multi(jd, s1, GUI_SML, GUI_ALL, gui_wht, gui_wht);
            gui_image(jd, "gui/help2.jpg", 6 * w / 16, 6 * h / 16);
        }
        gui_layout(id, 0, +1);
    }
    return id;
}

static int help_controls_enter(void)
{
    const char *s4 = _("Left and right mouse buttons rotate the view.");
    const char *s5 = _("Hold Shift for faster view rotation.");
    const char *s6 = _("Pause / Release Pointer");
    const char *s7 = _("Exit / Cancel Menu");
    const char *s8 = _("Chase View");
    const char *s9 = _("Lazy View");
    const char *sA = _("Manual View");
    const char *sB = _("Show FPS");
    const char *sC = _("Screenshot");
    const char *sD = _("Toggle Fullscreen");

    const char *k0 = _("Spacebar");
    const char *k1 = _("Escape");
    const char *k2 = SDL_GetKeyName(config_get_d(CONFIG_KEY_CAMERA_1));
    const char *k3 = SDL_GetKeyName(config_get_d(CONFIG_KEY_CAMERA_2));
    const char *k4 = SDL_GetKeyName(config_get_d(CONFIG_KEY_CAMERA_3));
    const char *k5 = SDL_GetKeyName(SDLK_F9);
    const char *k6 = SDL_GetKeyName(SDLK_F10);
    const char *k7 = SDL_GetKeyName(SDLK_F11);

    int id, jd;

    if ((id = gui_vstack(0)))
    {
        help_menu(id, HELP_CONT);

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
        if ((jd = gui_harray(id)))
        {
            gui_label(jd, sB, GUI_SML, GUI_SE, gui_wht, gui_wht);
            gui_label(jd, k5, GUI_SML, GUI_SW, gui_yel, gui_yel);
        }
        if ((jd = gui_harray(id)))
        {
            gui_label(jd, sC, GUI_SML, GUI_SE, gui_wht, gui_wht);
            gui_label(jd, k6, GUI_SML, GUI_SW, gui_yel, gui_yel);
        }
        if ((jd = gui_harray(id)))
        {
            gui_label(jd, sD, GUI_SML, GUI_SE, gui_wht, gui_wht);
            gui_label(jd, k7, GUI_SML, GUI_SW, gui_yel, gui_yel);
        }

        gui_space(id);
        gui_label(id, s4, GUI_SML, GUI_TOP, gui_wht, gui_wht);
        gui_label(id, s5, GUI_SML, GUI_BOT, gui_wht, gui_wht);

        gui_layout(id, 0, +1);
    }
    return id;
}

static int help_modes_enter(void)
{
    int id;

    if ((id = gui_vstack(0)))
    {
        help_menu(id, HELP_MODE);

        gui_label(id, _("Normal"), GUI_SML, GUI_TOP, 0, 0);
        gui_multi(id,
                  "Finish a level before the time runs out.\\"
                  "You need to collect coins in order to open the goal.",
                  GUI_SML, GUI_BOT, gui_wht, gui_wht);

        gui_space(id);

        gui_label(id, _("Practice"), GUI_SML, GUI_TOP, 0, 0);
        gui_multi(id, "Play without time limit or coin constraint.\\"
                        "Levels cannot be unlocked in this mode.",
                  GUI_SML, GUI_BOT, gui_wht, gui_wht);

        gui_space(id);

        gui_label(id, _("Challenge"), GUI_SML, GUI_TOP, 0, 0);
        gui_multi(id,
                  "Start playing from the first level of the set.\\"
                  "You start with only four balls, do not lose them.\\"
                  "Earn an extra ball for each 100 coins collected.",
                  GUI_SML, GUI_BOT, gui_wht, gui_wht);

        gui_layout(id, 0, +1);
    }
    return id;

}

static int help_techniques_enter(void)
{
    const char *s0 = _(
            "Corners can be used to jump.\\"
            "Get rolling and take aim\\"
            "at the angle. You may be able\\"
            "to reach new places.\\");
    const char *s1 = _(
            "Tilting in 2 directions increases\\"
            "the slope. Use the manual view\\"
            "and turn the camera by 45 degrees\\"
            "for best results.\\ ");

    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);

    int id, jd;

    if ((id = gui_vstack(0)))
    {
        help_menu(id, HELP_TECH);

        if ((jd = gui_hstack(id)))
        {
            gui_filler(jd);
            gui_multi(jd, s0, GUI_SML, GUI_ALL, gui_wht, gui_wht);
            gui_image(jd, "gui/help3.jpg", 6 * w / 16, 6 * h / 16);
        }
        gui_space(id);
        if ((jd = gui_hstack(id)))
        {
            gui_image(jd, "gui/help4.jpg", 6 * w / 16, 6 * h / 16);
            gui_multi(jd, s1, GUI_SML, GUI_ALL, gui_wht, gui_wht);
            gui_filler(jd);
        }
        gui_layout(id, 0, +1);
    }
    return id;
}

static int help_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return help_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return help_action(HELP_BACK);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_help = {
    help_enter,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    1, 0
};

struct state st_help_rules = {
    help_rules_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_click,
    NULL,
    help_buttn,
    1, 0
};

struct state st_help_controls = {
    help_controls_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_click,
    NULL,
    help_buttn,
    1, 0
};

struct state st_help_modes = {
    help_modes_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_click,
    NULL,
    help_buttn,
    1, 0
};

struct state st_help_techniques = {
    help_techniques_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_click,
    NULL,
    help_buttn,
    1, 0
};

