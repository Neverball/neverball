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
#include "game.h"
#include "util.h"
#include "level.h"
#include "audio.h"
#include "config.h"

#include "st_high.h"
#include "st_over.h"
#include "st_goal.h"
#include "st_level.h"

/*---------------------------------------------------------------------------*/

#define HIGH_OK 0
#define HIGH_BS 1

static int name_id;

/*---------------------------------------------------------------------------*/

static int high_action(int i)
{
    char player[MAXNAM];
	size_t l;

    config_get_name(player);
    l = strlen(player);

    switch (i)
    {
    case HIGH_OK:
        if (level_pass())
            return goto_state(&st_level);
        else
            return goto_state(&st_done);

    case HIGH_BS:
        if (l > 0)
        {
            player[l - 1] = 0;

            config_set_name(player);
            gui_set_label(name_id, player);
            gui_pulse(name_id, 1.2f);
        }
        break;

    default:
        if (l < MAXNAM - 1)
        {
            player[l + 0] = (char) i;
            player[l + 1] = 0;

            config_set_name(player);
            gui_set_label(name_id, player);
            gui_pulse(name_id, 1.2f);
        }
    }
    return 1;
}

static int high_enter(void)
{
    char player[MAXNAM];
    int id, jd, kd;

    config_get_name(player);

    if ((id = gui_vstack(0)))
    {
        name_id = gui_label(id, player,    GUI_MED, GUI_ALL, 0, gui_wht);
        gui_pulse(name_id, 1.2f);

        gui_space(id);
        gui_curr_score(id, curr_level());
        gui_space(id);

        if ((jd = gui_varray(id)))
        {
            if ((kd = gui_harray(jd)))
            {
                gui_state(kd, "G", GUI_SML, 'G', 0);
                gui_state(kd, "F", GUI_SML, 'F', 0);
                gui_state(kd, "E", GUI_SML, 'E', 0);
                gui_state(kd, "D", GUI_SML, 'D', 0);
                gui_state(kd, "C", GUI_SML, 'C', 0);
                gui_state(kd, "B", GUI_SML, 'B', 0);
                gui_state(kd, "A", GUI_SML, 'A', 0);
            }
            if ((kd = gui_harray(jd)))
            {
                gui_state(kd, "N", GUI_SML, 'N', 0);
                gui_state(kd, "M", GUI_SML, 'M', 0);
                gui_state(kd, "L", GUI_SML, 'L', 0);
                gui_state(kd, "K", GUI_SML, 'K', 0);
                gui_state(kd, "J", GUI_SML, 'J', 0);
                gui_state(kd, "I", GUI_SML, 'I', 0);
                gui_state(kd, "H", GUI_SML, 'H', 0);
            }
            if ((kd = gui_harray(jd)))
            {
                gui_state(kd, "U", GUI_SML, 'U', 0);
                gui_state(kd, "T", GUI_SML, 'T', 0);
                gui_state(kd, "S", GUI_SML, 'S', 0);
                gui_state(kd, "R", GUI_SML, 'R', 0);
                gui_state(kd, "Q", GUI_SML, 'Q', 0);
                gui_state(kd, "P", GUI_SML, 'P', 0);
                gui_state(kd, "O", GUI_SML, 'O', 0);
            }
            if ((kd = gui_harray(jd)))
            {
                gui_start(kd, "Ok", GUI_SML, HIGH_OK, 0);
                gui_state(kd, "<",  GUI_SML, HIGH_BS, 0);

                gui_state(kd, "Z", GUI_SML, 'Z', 0);
                gui_state(kd, "Y", GUI_SML, 'Y', 0);
                gui_state(kd, "X", GUI_SML, 'X', 0);
                gui_state(kd, "W", GUI_SML, 'W', 0);
                gui_state(kd, "V", GUI_SML, 'V', 0);
            }
        }
        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_most_coins(jd);
            gui_best_times(jd);
        }
        gui_layout(id, 0, 0);
    }
    set_most_coins(curr_level());
    set_best_times(curr_level());

    audio_play(AUD_SCORE, 1.f);

    return id;
}

static void high_leave(int id)
{
    gui_delete(id);
}

static void high_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void high_timer(int id, float dt)
{
    gui_timer(id, dt);
}   

static void high_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void high_stick(int id, int a, int v)
{
    if (config_tst(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int high_click(int b, int d)
{
    return (b < 0 && d == 1) ? high_action(gui_token(gui_click())) : 1;
}

static int high_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        goto_state(&st_over);
    return 1;
}

static int high_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return high_action(gui_token(gui_click()));
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_high = {
    high_enter,
    high_leave,
    high_paint,
    high_timer,
    high_point,
    high_stick,
    high_click,
    high_keybd,
    high_buttn,
    1, 0
};
