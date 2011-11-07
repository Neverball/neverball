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
#include "util.h"
#include "demo.h"
#include "progress.h"
#include "audio.h"
#include "config.h"

#include "game_common.h"

#include "st_done.h"
#include "st_level.h"
#include "st_name.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

enum
{
    DONE_OK = GUI_LAST
};

static int resume;

static int done_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case DONE_OK:
        return goto_state(&st_exit);

    case GUI_NAME:
        return goto_name(&st_done, &st_done, 0);

    case GUI_SCORE:
        gui_score_set(val);
        return goto_state(&st_done);
    }
    return 1;
}

static int done_gui(void)
{
    const char *s1 = _("New Set Record");
    const char *s2 = _("Set Complete");

    int id;

    int high = progress_set_high();

    if ((id = gui_vstack(0)))
    {
        int gid;

        if (high)
            gid = gui_label(id, s1, GUI_MED, gui_grn, gui_grn);
        else
            gid = gui_label(id, s2, GUI_MED, gui_blu, gui_grn);

        gui_space(id);
        gui_score_board(id, GUI_SCORE_COIN | GUI_SCORE_TIME, 1, high);
        gui_space(id);

        gui_start(id, _("Select Level"), GUI_SML, DONE_OK, 0);

        if (!resume)
            gui_pulse(gid, 1.2f);

        gui_layout(id, 0, 0);
    }

    set_score_board(set_score(curr_set(), SCORE_COIN), progress_score_rank(),
                    set_score(curr_set(), SCORE_TIME), progress_times_rank(),
                    NULL, -1);

    return id;
}

static int done_enter(struct state *st, struct state *prev)
{
    if (prev == &st_name)
        progress_rename(1);

    resume = (prev == &st_done);

    return done_gui();
}

static int done_keybd(int c, int d)
{
    if (d && config_tst_d(CONFIG_KEY_SCORE_NEXT, c))
        return done_action(GUI_SCORE, GUI_SCORE_NEXT(gui_score_get()));

    return 1;
}

static int done_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return done_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return done_action(DONE_OK, 0);
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
    done_keybd,
    done_buttn
};
