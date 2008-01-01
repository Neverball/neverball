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

#include "gui.h"
#include "game.h"
#include "util.h"
#include "levels.h"
#include "audio.h"
#include "config.h"
#include "demo.h"

#include "st_fall_out.h"
#include "st_save.h"
#include "st_over.h"
#include "st_start.h"
#include "st_level.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

#define FALL_OUT_NEXT 1
#define FALL_OUT_SAME 2
#define FALL_OUT_SAVE 3
#define FALL_OUT_BACK 4
#define FALL_OUT_OVER 5

static int be_back_soon;

static int fall_out_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case FALL_OUT_BACK:
        /* Fall through. */

    case FALL_OUT_OVER:
        level_stop();
        return goto_state(&st_over);

    case FALL_OUT_SAVE:
        be_back_soon = 1;

        level_stop();
        return goto_save(&st_fall_out, &st_fall_out);

    case FALL_OUT_NEXT:
        level_next();
        return goto_state(&st_level);

    case FALL_OUT_SAME:
        level_same();
        return goto_state(&st_level);
    }

    return 1;
}

static int fall_out_enter(void)
{
    int id, jd, kd;

    const struct level_game *lg = curr_lg();

    /* Reset hack. */
    be_back_soon = 0;

    if ((id = gui_vstack(0)))
    {
        kd = gui_label(id, _("Fall-out!"), GUI_LRG, GUI_ALL, gui_gry, gui_red);

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            int next_id = 0, retry_id = 0;

            next_id = gui_maybe(jd, _("Next Level"), FALL_OUT_NEXT,
                                lg->next_level != NULL);

            if (lg->dead)
            {
                gui_start(jd, _("Game Over"), GUI_SML, FALL_OUT_OVER, 0);
            }
            else
            {
                retry_id = gui_state(jd, _("Retry Level"), GUI_SML,
                                     FALL_OUT_SAME, 0);
            }

            gui_maybe(jd, _("Save Replay"), FALL_OUT_SAVE, demo_saved());

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
    /* audio_play(AUD_FALL, 1.0f); */

    config_clr_grab();

    return id;
}

static void fall_out_timer(int id, float dt)
{
    float g[3] = { 0.0f, -9.8f, 0.0f };

    if (time_state() < 2.f)
    {
        demo_play_step();
        game_step(g, dt, 0);
    }

    gui_timer(id, dt);
}

static int fall_out_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return fall_out_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return fall_out_action(FALL_OUT_BACK);
    }
    return 1;
}

static void fall_out_leave(int id)
{
    /* HACK:  don't run animation if only "visiting" a state. */
    st_fall_out.timer = be_back_soon ? shared_timer : fall_out_timer;

    gui_delete(id);
}

/*---------------------------------------------------------------------------*/

struct state st_fall_out = {
    fall_out_enter,
    fall_out_leave,
    shared_paint,
    fall_out_timer,
    shared_point,
    shared_stick,
    shared_click,
    NULL,
    fall_out_buttn,
    1, 0
};

