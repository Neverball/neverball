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
#include "game.h"
#include "level.h"
#include "audio.h"
#include "config.h"

#include "st_over.h"
#include "st_title.h"

/*---------------------------------------------------------------------------*/

static int over_enter(void)
{
    int id;

    if ((id = gui_label(0, "GAME OVER", GUI_LRG, GUI_ALL, gui_gry, gui_red)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    audio_music_fade_out(2.0f);
    audio_play(AUD_OVER, 1.f);

    config_clr_grab();

    return id;
}

static void over_leave(int id)
{
    level_exit(NULL, 0);
    gui_delete(id);
    set_free();
}

static void over_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void over_timer(int id, float dt)
{
    if (dt > 0.f && time_state() > 3.f)
        goto_state(&st_title);

    gui_timer(id, dt);
    audio_timer(dt);
}

static int over_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int over_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_title) : 1;
}

static int over_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_title);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_title);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_title);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_over = {
    over_enter,
    over_leave,
    over_paint,
    over_timer,
    NULL,
    NULL,
    over_click,
    over_keybd,
    over_buttn,
    1, 0
};
