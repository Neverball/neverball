/*
 * Copyright (C) 2003 Robert Kooima - 2008 Byron Johnson
 * Part of the Neverball Project http://icculus.org/neverball/
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
#include "back.h"
#include "part.h"
#include "game.h"
#include "audio.h"
#include "config.h"

#include "st_balt.h"

extern struct state st_conf;
extern struct state st_null;

/*---------------------------------------------------------------------------*/

enum {
    BALT_BACK = 1,
    BALT_GOLF,
    BALT_BILL,
    BALT_CRAZ
};

static int balt_action(int i)
{
    int r = 1;

    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case BALT_BACK:
        goto_state(&st_conf);
        break;

    case BALT_GOLF:
        goto_state(&st_null);
        config_set_s(CONFIG_BALL_GAMMA, "0.78");
        goto_state(&st_balt);
        break;

    case BALT_BILL:
        goto_state(&st_null);
        config_set_s(CONFIG_BALL_GAMMA, "1.00");
        goto_state(&st_balt);
        break;

    case BALT_CRAZ:
        goto_state(&st_null);
        config_set_s(CONFIG_BALL_GAMMA, "1.50");
        goto_state(&st_balt);
        break;

    default:
        break;
    }

    return r;
}

static int balt_enter(void)
{
    int id, jd;

    char gamma[MAXNAM];

    config_get_s(CONFIG_BALL_GAMMA, gamma, MAXNAM);

    back_init("back/gui.png", config_get_d(CONFIG_GEOMETRY));

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_harray(id)))
        {
            gui_space(jd);
            gui_space(jd);
            gui_start(jd, _("Back"),                     GUI_SML, BALT_BACK, 0);
        }

        gui_space(id);
        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_state(jd, _("Billiards"),                GUI_SML, BALT_BILL,
                          strcmp(gamma, "1.00")  == 0 ||
                          strcmp(gamma, "1.0")   == 0);
            gui_state(jd, _("Golf Balls"),               GUI_SML, BALT_GOLF,
                          strcmp(gamma, "0.78")  == 0);
        }

        if ((jd = gui_harray(id)))
        {
            gui_state(jd, _("Crazy Balls"),              GUI_SML, BALT_CRAZ,
                          strcmp(gamma, "1.50") == 0 ||
                          strcmp(gamma, "1.5")  == 0);
        }

        gui_layout(id, 0, 0);
    }

    audio_music_fade_to(0.5f, "bgm/inter.ogg");

    return id;
}

static void balt_leave(int id)
{
    back_free();
    gui_delete(id);
}

static void balt_paint(int id, float st)
{
    config_push_persp((float) config_get_d(CONFIG_VIEW_FOV), 0.1f, FAR_DIST);
    {
        back_draw(0);
    }
    config_pop_matrix();
    gui_paint(id);
}

static void balt_timer(int id, float dt)
{
    gui_timer(id, dt);
}

static void balt_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void balt_stick(int id, int a, int v)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int balt_click(int b, int d)
{
    if (b < 0 && d == 1)
        return balt_action(gui_token(gui_click()));
    return 1;
}

static int balt_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_conf) : 1;
}

static int balt_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return balt_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_conf);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_conf);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/


struct state st_balt = {
    balt_enter,
    balt_leave,
    balt_paint,
    balt_timer,
    balt_point,
    balt_stick,
    NULL,
    balt_click,
    balt_keybd,
    balt_buttn,
    1, 0
};

