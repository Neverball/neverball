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
#include "hud.h"
#include "set.h"
#include "game.h"
#include "demo.h"
#include "level.h"
#include "audio.h"
#include "config.h"

#include "st_demo.h"
#include "st_title.h"

/*---------------------------------------------------------------------------*/

static float replay_time;
static float global_time;

/*---------------------------------------------------------------------------*/

static int demo_enter(void)
{
    int id;

    if ((id = gui_label(0, "Demo", GUI_LRG, GUI_ALL, gui_blu, gui_grn)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    global_time = -1.f;
    replay_time =  0.f;

    demo_replay_init();
    game_set_fly(0.f);

    return id;
}

static void demo_leave(int id)
{
    demo_finish();
    gui_delete(id);
}

static void demo_paint(int id, float st)
{
    game_draw(0, st);
    hud_paint();

    if (time_state() < 1.f)
        gui_paint(id);
}

static void demo_timer(int id, float dt)
{
    float t;

    gui_timer(id, dt);
    global_time += dt;

    /* Spin or skip depending on how fast the demo wants to run. */

    while (replay_time < global_time)
        if (demo_replay_step(&t))
        {
            hud_timer(t);
            replay_time += t;
        }
        else 
        {
            goto_state(&st_demo_end);
            break;
        }
}

static int demo_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        goto_state(&st_demo_end);
    return 1;
}

static int demo_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_demo_end);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int demo_end_enter(void)
{
    int id;

    if ((id = gui_label(0, "Demo Ends", GUI_LRG, GUI_ALL, gui_blk, gui_red)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    audio_music_fade(3.f);

    return id;
}

static void demo_end_leave(int id)
{
    audio_music_stop();
    gui_delete(id);
    set_free();
}

static void demo_end_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void demo_end_timer(int id, float dt)
{
    if (dt > 0.f && time_state() > 3.f)
        goto_state(&st_title);

    gui_timer(id, dt);
}

static int demo_end_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int demo_end_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_title) : 1;
}

static int demo_end_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_title);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_title);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_title);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_demo = {
    demo_enter,
    demo_leave,
    demo_paint,
    demo_timer,
    NULL,
    NULL,
    NULL,
    demo_keybd,
    demo_buttn,
    0
};

struct state st_demo_end = {
    demo_end_enter,
    demo_end_leave,
    demo_end_paint,
    demo_end_timer,
    NULL,
    NULL,
    demo_end_click,
    demo_end_keybd,
    demo_end_buttn,
    1, 0
};
