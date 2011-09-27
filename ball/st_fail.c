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

#include "util.h"
#include "progress.h"
#include "demo.h"
#include "audio.h"
#include "gui.h"
#include "config.h"
#include "video.h"

#include "game_common.h"
#include "game_server.h"
#include "game_client.h"

#include "st_save.h"
#include "st_fail.h"
#include "st_level.h"
#include "st_play.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

enum
{
    FAIL_NEXT = GUI_LAST,
    FAIL_SAME,
    FAIL_SAVE,
    FAIL_BACK,
    FAIL_OVER
};

static int resume;
static int status;

static int fail_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case FAIL_BACK:
    case FAIL_OVER:
        progress_stop();
        return goto_state(&st_exit);

    case FAIL_SAVE:
        progress_stop();
        return goto_save(&st_fail, &st_fail);

    case FAIL_NEXT:
        if (progress_next())
            return goto_state(&st_level);
        break;

    case FAIL_SAME:
        if (progress_same())
            return goto_state(&st_level);
        break;
    }

    return 1;
}

static int fail_gui(void)
{
    int id, jd, kd;

    const char *label = "";

    if (status == GAME_FALL)
        label = _("Fall-out!");
    else if (status == GAME_TIME)
        label = _("Time's Up!");

    if ((id = gui_vstack(0)))
    {
        kd = gui_label(id, label, GUI_LRG, GUI_ALL, gui_gry, gui_red);

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            if (progress_dead())
                gui_start(jd, _("Exit"), GUI_SML, FAIL_OVER, 0);

            if (progress_next_avail())
                gui_start(jd, _("Next Level"),  GUI_SML, FAIL_NEXT, 0);

            if (progress_same_avail())
                gui_start(jd, _("Retry Level"), GUI_SML, FAIL_SAME, 0);

            if (demo_saved())
                gui_state(jd, _("Save Replay"), GUI_SML, FAIL_SAVE, 0);
        }

        gui_space(id);

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);
    }

    return id;
}

static int fail_enter(struct state *st, struct state *prev)
{
    audio_music_fade_out(2.0f);
    video_clr_grab();

    /* Check if we came from a known previous state. */

    resume = (prev == &st_fail || prev == &st_save);

    /* Note the current status if we got here from elsewhere. */

    if (!resume)
        status = curr_status();

    return fail_gui();
}

static void fail_timer(int id, float dt)
{
    if (status == GAME_FALL)
    {
        if (!resume && time_state() < 2.f)
        {
            game_server_step(dt);
            game_client_sync(demo_fp);
            game_client_blend(game_server_blend());
        }
    }

    gui_timer(id, dt);
}

static int fail_keybd(int c, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_KEY_RESTART, c) && progress_same_avail())
        {
            if (progress_same())
                goto_state(&st_play_ready);
        }
    }
    return 1;
}

static int fail_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return fail_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return fail_action(FAIL_BACK, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_fail = {
    fail_enter,
    shared_leave,
    shared_paint,
    fail_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    fail_keybd,
    fail_buttn
};

