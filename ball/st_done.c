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
#include "levels.h"
#include "audio.h"
#include "config.h"
#include "st_shared.h"

#include "st_done.h"
#include "st_start.h"
#include "st_name.h"

/*---------------------------------------------------------------------------*/

#define DONE_OK   1
#define DONE_NAME 2

/* Bread crumbs. */

static int new_name;

static int done_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case DONE_OK:
        return goto_state(&st_start);

    case DONE_NAME:
        new_name = 1;
        return goto_name(&st_done, &st_done, 0);
    }
    return 1;
}

static int done_enter(void)
{
    const char *s1 = _("New Set Record");
    const char *s2 = _("Set Complete");

    int id, jd;

    int high = (curr_lg()->times_rank < 3) || (curr_lg()->score_rank < 3);

    if (new_name)
    {
        level_update_player_name();
        new_name = 0;
    }

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
            gui_most_coins(jd, 1);
            gui_best_times(jd, 1);
        }

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            /* FIXME, I'm ugly. */

            if (high)
               gui_state(jd, _("Change Player Name"), GUI_SML, DONE_NAME, 0);

            gui_start(jd, _("OK"), GUI_SML, DONE_OK, 0);
        }

        gui_layout(id, 0, 0);
        gui_pulse(gid, 1.2f);
    }

    set_best_times(&curr_set()->time_score, curr_lg()->times_rank, 0);
    set_most_coins(&curr_set()->coin_score, curr_lg()->score_rank);

    return id;
}

static int done_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return done_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return done_action(DONE_OK);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_done = {
    done_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    NULL,
    done_buttn,
    1, 0
};
