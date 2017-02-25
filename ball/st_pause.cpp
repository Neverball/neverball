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

#include "gui.h"
#include "config.h"
#include "video.h"
#include "progress.h"
#include "level.h"
#include "audio.h"
#include "hud.h"

#include "game_common.h"

#include "st_play.h"
#include "st_level.h"
#include "st_pause.h"
#include "st_shared.h"

enum
{
    PAUSE_CONTINUE = GUI_LAST,
    PAUSE_RESTART,
    PAUSE_EXIT
};

static struct state *st_continue;

/*---------------------------------------------------------------------------*/

static int pause_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case PAUSE_CONTINUE:
        SDL_PauseAudio(0);
        video_set_grab(0);
        return goto_state(st_continue);

    case PAUSE_RESTART:
        if (progress_same())
        {
            SDL_PauseAudio(0);
            video_set_grab(1);
            return goto_state(&st_play_ready);
        }
        break;

    case PAUSE_EXIT:
        progress_stat(GAME_NONE);
        progress_stop();
        SDL_PauseAudio(0);
        audio_music_stop();
        return goto_state(&st_exit);
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

static int pause_gui(void)
{
    int id, jd, title_id;

    /* Build the pause GUI. */

    if ((id = gui_vstack(0)))
    {
        title_id = gui_label(id, _("Paused"), GUI_LRG, 0, 0);

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_state(jd, _("Quit"), GUI_SML, PAUSE_EXIT, 0);

            if (progress_same_avail())
                gui_state(jd, _("Restart"), GUI_SML, PAUSE_RESTART, 0);

            gui_start(jd, _("Continue"), GUI_SML, PAUSE_CONTINUE, 0);
        }

        gui_pulse(title_id, 1.2f);
        gui_layout(id, 0, 0);
    }

    return id;
}

static int pause_enter(struct state *st, struct state *prev)
{
    st_continue = prev;

    video_clr_grab();
    SDL_PauseAudio(1);

    hud_update(0);

    return pause_gui();
}

static void pause_paint(int id, float t)
{
    shared_paint(id, t);
    hud_paint();
}

static void pause_timer(int id, float dt)
{
    gui_timer(id, dt);
    hud_timer (dt);
}

static int pause_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
            return pause_action(PAUSE_CONTINUE, 0);

        if (config_tst_d(CONFIG_KEY_RESTART, c) && progress_same_avail())
            return pause_action(PAUSE_RESTART, 0);
    }
    return 1;
}

static int pause_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return pause_action(gui_token(active), gui_value(active));

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b) ||
            config_tst_d(CONFIG_JOYSTICK_BUTTON_START, b))
            return pause_action(PAUSE_CONTINUE, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_pause = {
    pause_enter,
    shared_leave,
    pause_paint,
    pause_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    pause_keybd,
    pause_buttn
};
