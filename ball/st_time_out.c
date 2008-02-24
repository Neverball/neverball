/*
 * Copyright (C) 2007 Robert Kooima
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

#include "game.h"
#include "util.h"
#include "levels.h"
#include "demo.h"
#include "audio.h"
#include "gui.h"
#include "config.h"

#include "st_over.h"
#include "st_start.h"
#include "st_save.h"
#include "st_time_out.h"
#include "st_level.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

#define TIME_OUT_NEXT 1
#define TIME_OUT_SAME 2
#define TIME_OUT_SAVE 3
#define TIME_OUT_BACK 4
#define TIME_OUT_OVER 5

static int time_out_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case TIME_OUT_BACK:
        /* Fall through. */

    case TIME_OUT_OVER:
        level_stop();
        return goto_state(&st_over);

    case TIME_OUT_SAVE:
        level_stop();
        return goto_save(&st_time_out, &st_time_out);

    case TIME_OUT_NEXT:
        level_next();
        return goto_state(&st_level);

    case TIME_OUT_SAME:
        level_same();
        return goto_state(&st_level);
    }

    return 1;
}

static int time_out_enter(void)
{
    int id, jd, kd;

    const struct level_game *lg = curr_lg();

    if ((id = gui_vstack(0)))
    {
        kd = gui_label(id, _("Time's Up!"), GUI_LRG, GUI_ALL, gui_gry, gui_red);

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            int next_id = 0, retry_id = 0;

            next_id = gui_maybe(jd, _("Next Level"),  TIME_OUT_NEXT,
                                lg->next_level != NULL);

            if (lg->dead)
                gui_start(jd, _("Game Over"), GUI_SML, TIME_OUT_OVER, 0);
            else
            {
                retry_id = gui_state(jd, _("Retry Level"), GUI_SML,
                                     TIME_OUT_SAME, 0);
            }

            gui_maybe(jd, _("Save Replay"), TIME_OUT_SAVE, demo_saved());

            /* Default is next if the next level is newly unlocked. */

            if (next_id && lg->unlock)
                gui_focus(next_id);
            else if (retry_id)
                gui_focus(retry_id);
        }
        gui_space(id);

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);
    }

    audio_music_fade_out(2.0f);
    /* audio_play(AUD_TIME, 1.0f); */

    config_clr_grab();

    return id;
}

static int time_out_keybd(int c, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_KEY_RESTART, c) && !curr_lg()->dead)
            return time_out_action(TIME_OUT_SAME);
    }
    return 1;
}

static int time_out_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return time_out_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return time_out_action(TIME_OUT_BACK);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_time_out = {
    time_out_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    time_out_keybd,
    time_out_buttn,
    1, 0
};

