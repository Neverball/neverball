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
#include "st_save.h"
#include "st_level.h"

/*---------------------------------------------------------------------------*/

#define FAIL_RETRY 0
#define FAIL_SAVE  1

static int fail_action(int i)
{
    switch (i)
    {
    case FAIL_RETRY:
        if (level_exit(NULL, 0))
            return goto_state(&st_level);
        else
            return goto_state(&st_over);

    case FAIL_SAVE:
        return goto_state(&st_save);
    }
    return 1;
}

static int fall_out_enter(void)
{
    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        kd = gui_label(id, "Fall-out!", GUI_LRG, GUI_ALL, gui_gry, gui_red);
    
        if ((jd = gui_harray(id)))
        {
            gui_state(jd, "Save Replay", GUI_SML, FAIL_SAVE, 0);

            if (level_dead())
                gui_start(jd, "Main Menu",   GUI_SML, FAIL_RETRY, 0);
            else
                gui_start(jd, "Retry Level", GUI_SML, FAIL_RETRY, 0);
        }

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);
    }

    audio_music_fade_out(2.0f);
    audio_play(AUD_FALL, 1.0f);

    config_clr_grab();

    return id;
}

static void fall_out_leave(int id)
{
    gui_delete(id);
}

static void fall_out_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void fall_out_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void fall_out_stick(int id, int a, int v)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int fall_out_click(int b, int d)
{
    if (b <= 0 && d == 1)
        return fail_action(gui_token(gui_click()));
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
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return fall_out_click(0, 1);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int time_out_enter(void)
{
    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        kd = gui_label(id, "Time's Up!", GUI_LRG, GUI_ALL, gui_gry, gui_red);
    
        if ((jd = gui_harray(id)))
        {
            gui_state(jd, "Save Replay", GUI_SML, FAIL_SAVE, 0);

            if (level_dead())
                gui_start(jd, "Main Menu",   GUI_SML, FAIL_RETRY, 0);
            else
                gui_start(jd, "Retry Level", GUI_SML, FAIL_RETRY, 0);
        }

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);
    }

    audio_music_fade_out(2.0f);
    audio_play(AUD_TIME, 1.0f);

    config_clr_grab();

    return id;
}

static void time_out_leave(int id)
{
    gui_delete(id);
}

static void time_out_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void time_out_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void time_out_stick(int id, int a, int v)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int time_out_click(int b, int d)
{
    if (b <= 0 && d == 1)
        return fail_action(gui_token(gui_click()));
    return 1;
}

static void time_out_timer(int id, float dt)
{
    gui_timer(id, dt);
    audio_timer(dt);
}

static int time_out_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        goto_state(&st_over);
    return 1;
}

static int time_out_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return time_out_click(0, 1);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
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
    fall_out_point,
    fall_out_stick,
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
    time_out_point,
    time_out_stick,
    time_out_click,
    time_out_keybd,
    time_out_buttn,
    1, 0
};
