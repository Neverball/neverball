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
#include "set.h"
#include "progress.h"
#include "audio.h"
#include "config.h"
#include "video.h"
#include "demo.h"

#include "game_common.h"

#include "st_over.h"
#include "st_level.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

static int over_gui(void)
{
    int id;

    if ((id = gui_label(0, _("GAME OVER"), GUI_LRG, gui_gry, gui_red)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    return id;
}

static int over_enter(struct state *st, struct state *prev)
{
    audio_music_fade_out(2.0f);
    audio_play(AUD_OVER, 1.f);

    video_clr_grab();

    return over_gui();
}

static void over_timer(int id, float dt)
{
    if (time_state() > 3.f)
        goto_state(&st_exit);

    gui_timer(id, dt);
}

static int over_click(int b, int d)
{
    return (b == SDL_BUTTON_LEFT && d == 1) ? goto_state(&st_exit) : 1;
}

static int over_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
            return goto_state(&st_exit);
    }
    return 1;
}

static int over_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b) ||
            config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_exit);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_over = {
    over_enter,
    shared_leave,
    shared_paint,
    over_timer,
    NULL,
    NULL,
    NULL,
    over_click,
    over_keybd,
    over_buttn
};
