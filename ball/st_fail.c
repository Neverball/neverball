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
#include "game.h"
#include "demo.h"
#include "level.h"
#include "audio.h"
#include "config.h"

#include "st_fail.h"
#include "st_over.h"
#include "st_level.h"

/*---------------------------------------------------------------------------*/

static int fall_out_enter(void)
{
    int id;

    if ((id = gui_label(0, "Fall-out!", GUI_LRG, GUI_ALL, gui_blk, gui_red)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    audio_music_fade_out(2.0f);
    audio_play(AUD_FALL, 1.0f);

    return id;
}

static void fall_out_leave(int id)
{
    demo_finish();
    gui_delete(id);
}

static void fall_out_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static int fall_out_click(int b, int d)
{
    if (b <= 0 && d == 1)
    {
        if (level_fail())
            goto_state(&st_level);
        else
            goto_state(&st_over);
    }
    return 1;
}

static void fall_out_timer(int id, float dt)
{
    float g[3] = { 0.0f, -9.8f, 0.0f };

    if (time_state() < 2.f)
    {
        game_step(g, dt, 0);
        demo_play_step(dt);
    }
    else
        fall_out_click(0, 1);

    gui_timer(id, dt);
    audio_timer(dt);
}

static int fall_out_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        goto_state(&st_over);
    return 1;
}

static int fall_out_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return fall_out_click(0, 1);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int time_out_enter(void)
{
    int id;

    if ((id = gui_label(0, "Time's Up!", GUI_LRG, GUI_ALL,gui_blk, gui_red)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    audio_music_fade_out(2.0f);
    audio_play(AUD_TIME, 1.0f);

    return id;
}

static void time_out_leave(int id)
{
    demo_finish();
    gui_delete(id);
}

static void time_out_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static int time_out_click(int b, int d)
{
    if (b <= 0 && d == 1)
    {
        if (level_fail())
            goto_state(&st_level);
        else
            goto_state(&st_over);
    }
    return 1;
}

static void time_out_timer(int id, float dt)
{
    float g[3] = { 0.0f, -9.8f, 0.0f };

    if (time_state() < 2.f)
    {
        game_step(g, dt, 0);
        demo_play_step(dt);
    }
    else
        time_out_click(0, 1);

    gui_timer(id, dt);
    audio_timer(dt);
}

static int time_out_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return time_out_click(0, 1);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_fall_out = {
    fall_out_enter,
    fall_out_leave,
    fall_out_paint,
    fall_out_timer,
    NULL,
    NULL,
    fall_out_click,
    fall_out_keybd,
    fall_out_buttn,
    1, 0
};

struct state st_time_out = {
    time_out_enter,
    time_out_leave,
    time_out_paint,
    time_out_timer,
    NULL,
    NULL,
    time_out_click,
    NULL,
    time_out_buttn,
    1, 0
};
