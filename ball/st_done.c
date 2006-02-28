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
#include "st_start.h"
#include "st_name.h"

/*---------------------------------------------------------------------------*/

#define DONE_OK   1
#define DONE_NAME 2

extern struct state st_done_bis;

static int high;
static int time_i;
static int coin_i;

static int done_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case DONE_OK:
        return goto_state(&st_start);

    case DONE_NAME:
	return goto_name(&st_done_bis);
    }
    return 1;
}

static int done_init(int * gidp)
{
    const char *s1 = _("New Challenge Record");
    const char *s2 = _("Challenge Complete");

    int id, jd;

    if ((id = gui_vstack(0)))
    {
        int gid;

        if (high)
            gid = gui_label(id, s1, GUI_MED, GUI_ALL, gui_grn, gui_grn);
        else
            gid = gui_label(id, s2, GUI_MED, GUI_ALL, gui_blu, gui_grn);

	if (gidp)
	    *gidp = gid;

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_most_coins(jd, 3, 1);
            gui_best_times(jd, 3, 1);
        }

        gui_space(id);
	
	if ((jd = gui_harray(id)))
	{
            if (high)
	       gui_state(jd, _("Change Player Name"), GUI_SML, DONE_NAME, 0);
            gui_start(jd, _("OK"), GUI_SML, DONE_OK, 0);
	}

        gui_layout(id, 0, 0);
    }

    set_most_coins(0, coin_i);
    set_best_times(0, time_i);

    return id;
}

static int done_enter(void)
{
    int gid, r;
    
    time_i = 3;
    coin_i = 3;
    high   = level_done(&time_i, &coin_i);
    
    r = done_init(&gid);
    gui_pulse(gid, 1.2f);    
    return r;
}

static int done_bis_enter(void)
{
    char player[MAXNAM];
    config_get_s(CONFIG_PLAYER, player, MAXNAM);
    level_name(0, player, time_i, coin_i);
    return done_init(NULL);
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

static int done_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return done_click(0, 1);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return done_action(DONE_OK);
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
    NULL,
    done_buttn,
    1, 0
};

struct state st_done_bis = {
    done_bis_enter,
    done_leave,
    done_paint,
    done_timer,
    done_point,
    done_stick,
    done_click,
    NULL,
    done_buttn,
    1, 0
};

