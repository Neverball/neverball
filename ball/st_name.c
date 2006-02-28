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
#include "util.h"
#include "audio.h"
#include "config.h"
#include "game.h"

#include "st_name.h"

/*---------------------------------------------------------------------------*/

extern struct state st_name;

static struct state * next_state;
static char player[MAXNAM];

int goto_name(struct state * nextstate)
{
    next_state = nextstate;
    return goto_state(&st_name);
}


#define NAME_BACK   2
#define NAME_CANCEL 3
#define NAME_OK     4

static int name_id;

static int name_action(int i)
{
    size_t l;

    audio_play(AUD_MENU, 1.0f);

    l = strlen(player);

    switch (i)
    {
    case NAME_OK:
	if (l == 0)
	   return 1;
        config_set_s(CONFIG_PLAYER, player);
	/* no break, thus continue */
	
    case NAME_BACK:
    case NAME_CANCEL:
	return goto_state(next_state);
	
    case GUI_CL:
        gui_keyboard_lock();
        break;

    case GUI_BS:
        if (l > 0)
        {
            player[l - 1] = '\0';
            gui_set_label(name_id, player);
        }
        break;

    default:
        if (l < MAXNAM - 1)
        {
            player[l + 0] = gui_keyboard_char((char) i);
            player[l + 1] = '\0';
            gui_set_label(name_id, player);
        }
    }
    return 1;
}

static int name_enter(void)
{
    int id, jd;

    config_get_s(CONFIG_PLAYER, player, MAXNAM);

    if ((id = gui_vstack(0)))
    {
        int gid;

	gid = gui_label(id, _("Player Name"), GUI_MED, GUI_ALL, 0, 0);

	gui_space(id);
        gui_space(id);
	
	name_id = gui_label(id, player, GUI_MED, GUI_ALL, gui_yel, gui_yel);

        gui_space(id);

	if ((jd = gui_harray(id)))
	{
	    gui_state(jd, _("Cancel"), GUI_SML, NAME_CANCEL, 0);
	    gui_start(jd, _("OK"),     GUI_SML, NAME_OK,     0);
	}
	gui_keyboard(id);

	gui_layout(id, 0, 0);
    }

    return id;
}

static void name_leave(int id)
{
    gui_delete(id);
}

static void name_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void name_timer(int id, float dt)
{
    gui_timer(id, dt);
    audio_timer(dt);
}

static void name_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void name_stick(int id, int a, int v)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int name_click(int b, int d)
{
    if (b <= 0 && d == 1)
        return name_action(gui_token(gui_click()));
    return 1;
}

static int name_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return name_click(0, 1);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
	    name_action(NAME_BACK);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_name = {
    name_enter,
    name_leave,
    name_paint,
    name_timer,
    name_point,
    name_stick,
    name_click,
    NULL,
    name_buttn,
    1, 0
};

