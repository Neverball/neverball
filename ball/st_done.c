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

#include <string.h>

#include "gui.h"
#include "set.h"
#include "game.h"
#include "util.h"
#include "demo.h"
#include "level.h"
#include "audio.h"
#include "config.h"

#include "st_done.h"
#include "st_title.h"

/*---------------------------------------------------------------------------*/

#define DONE_OK 2

static int high;
static int time_i;
static int coin_i;

static int done_action(int i)
{
    char player[MAXNAM];
    size_t l;

    audio_play(AUD_MENU, 1.0f);

    config_get_s(CONFIG_PLAYER, player, MAXNAM);
    l = strlen(player);

    switch (i)
    {
    case DONE_OK:
        level_free();
        return goto_state(&st_title);

    case GUI_CL:
        gui_keyboard_lock();
        break;

    case GUI_BS:
        if (l > 0)
        {
            player[l - 1] = 0;

            config_set_s(CONFIG_PLAYER, player);
            level_name(0, player, time_i, coin_i);
            set_most_coins(0, 4);
            set_best_times(0, 4);
        }
        break;

    default:
        if (l < MAXNAM - 1)
        {
            player[l + 0] = gui_keyboard_char((char) i);
            player[l + 1] = 0;

            config_set_s(CONFIG_PLAYER, player);
            level_name(0, player, time_i, coin_i);
            set_most_coins(0, 4);
            set_best_times(0, 4);
        }
    }
    return 1;
}

static int done_enter(void)
{
    const char *s1 = "New Set Record";
    const char *s2 = "Set Complete";

    int id, jd;

    time_i = 3;
    coin_i = 3;
    high   = level_done(&time_i, &coin_i);

    if ((id = gui_vstack(0)))
    {
        int gid;

        if (high)
            gid = gui_label(id, s1, GUI_MED, GUI_ALL, gui_grn, gui_grn);
        else
            gid = gui_label(id, s2, GUI_MED, GUI_ALL, gui_blu, gui_grn);

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_most_coins(jd, 4, coin_i);
            gui_best_times(jd, 4, time_i);
        }

        gui_space(id);
        gui_start(id, "Main Menu", GUI_SML, DONE_OK, 0);

        if (high) gui_keyboard(id);

        gui_layout(id, 0, 0);
        gui_pulse(gid, 1.2f);
    }

    set_most_coins(0, 4);
    set_best_times(0, 4);

    return id;
}

static void done_leave(int id)
{
    gui_delete(id);
}

static void done_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void done_timer(int id, float dt)
{
    gui_timer(id, dt);
    audio_timer(dt);
}

static void done_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void done_stick(int id, int a, int v)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int done_click(int b, int d)
{
    if (b <= 0 && d == 1)
        return done_action(gui_token(gui_click()));
    return 1;
}

static int done_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        goto_state(&st_title);
    return 1;
}

static int done_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return done_click(0, 1);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_title);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_done = {
    done_enter,
    done_leave,
    done_paint,
    done_timer,
    done_point,
    done_stick,
    done_click,
    done_keybd,
    done_buttn,
    1, 0
};

