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
#include "solid.h"
#include "config.h"

#include "st_demo.h"
#include "st_title.h"

/*---------------------------------------------------------------------------*/

#define DEMO_BACK (MAXDEMO+1)
#define DEMO_NEXT (MAXDEMO+2)
#define DEMO_PREV (MAXDEMO+3)

#define DEMO_LINE 4
#define DEMO_STEP 8

static int first = 0;
static int total = 0;

static float replay_time;
static float global_time;

/*---------------------------------------------------------------------------*/

static int demo_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case DEMO_BACK:
        return goto_state(&st_title);

    case DEMO_NEXT:
        first += DEMO_STEP;
        return goto_state(&st_demo);
        break;

    case DEMO_PREV:
        first -= DEMO_STEP;
        return goto_state(&st_demo);
        break;

    default:
        if (level_replay(demo_name(i)))
            return goto_state(&st_demo_play);
    }
    return 1;
}

static void demo_replay(int id, int i)
{
    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);
    int jd;

    if ((jd = gui_vstack(id)))
    {
        gui_space(jd);

        gui_image(jd, demo_shot(i), w / 6, h / 6);
        gui_state(jd, demo_name(i), GUI_SML, i, 0);

        gui_clock(jd, demo_clock(i), GUI_SML, GUI_TOP);
        gui_count(jd, demo_coins(i), GUI_SML, GUI_BOT);

        gui_active(jd, i, 0);
    }
}

static int demo_enter(void)
{
    int i, j;
    int id, jd, kd, ld;

    total = demo_scan();

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_hstack(id)))
        {
            if ((kd = gui_vstack(jd)))
            {
                gui_filler(kd);
                if (first + DEMO_STEP < total)
                    gui_state(kd, _("Next"), GUI_SML, DEMO_NEXT, 0);
                else
                    gui_state(kd, _("Back"), GUI_SML, DEMO_BACK, 0);
            }

            gui_filler(jd);
            ld = gui_label(jd, _("Select Replay"), GUI_MED, GUI_ALL, 0,0);
            gui_filler(jd);

            if ((kd = gui_vstack(jd)))
            {
                gui_filler(kd);
                if (first > 0)
                    gui_start(kd, _("Prev"), GUI_SML, DEMO_PREV, 0);
                else
                    gui_start(kd, _("Back"), GUI_SML, DEMO_BACK, 0);
            }

            gui_pulse(ld, 1.2f);
        }
        if ((jd = gui_varray(id)))
            for (i = first; i < first + DEMO_STEP && i < total; i += DEMO_LINE)
                if ((kd = gui_harray(jd)))
                {
                    for (j = i + DEMO_LINE - 1; j >= i; j--)
                        if (j < total)
                            demo_replay(kd, j);
                        else
                            gui_filler(kd);
                }

        gui_layout(id, 0, 0);
    }

    audio_music_fade_to(0.5f, "bgm/inter.ogg");

    return id;
}

static void demo_leave(int id)
{
    gui_delete(id);
}

static void demo_paint(int id, float st)
{
    game_draw(0, st);
    config_pop_matrix();
    gui_paint(id);
}

static void demo_timer(int id, float dt)
{
    gui_timer(id, dt);
    audio_timer(dt);
}

static void demo_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void demo_stick(int id, int a, int v)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int demo_click(int b, int d)
{
    if (b <= 0 && d == 1)
        return demo_action(gui_token(gui_click()));
    return 1;
}

static int demo_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return demo_click(0, 1);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return demo_action(DEMO_BACK);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int demo_play_enter(void)
{
    int id;

    if ((id = gui_label(0, _("Replay"), GUI_LRG, GUI_ALL, gui_blu, gui_grn)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    global_time = -1.f;
    replay_time =  0.f;

    game_set_fly(0.f);

    return id;
}

static void demo_play_leave(int id)
{
    gui_delete(id);
}

static void demo_play_paint(int id, float st)
{
    game_draw(0, st);
    hud_paint();

    if (time_state() < 1.f)
        gui_paint(id);
}

static void demo_play_timer(int id, float dt)
{
    float t;

    game_step_fade(dt);
    gui_timer(id, dt);
    audio_timer(dt);

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

static int demo_play_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_demo_end);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

#define DEMO_KEEP 0
#define DEMO_DEL  1

static int demo_end_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    if (i == DEMO_DEL)
        return goto_state(&st_demo_del);

    else
    {
        demo_replay_stop(0);
        return goto_state(&st_demo);
    }
}

static int demo_end_enter(void)
{
    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        kd = gui_label(id, _("Replay Ends"), GUI_MED, GUI_ALL, gui_gry, gui_red);

        if ((jd = gui_harray(id)))
        {
            gui_state(jd, _("Delete"), GUI_SML, DEMO_DEL,  0);
            gui_start(jd, _("Keep"),   GUI_SML, DEMO_KEEP, 1);
        }

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);
    }
    audio_music_fade_out(2.0f);

    return id;
}

static void demo_end_leave(int id)
{
    gui_delete(id);
}

static void demo_end_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void demo_end_timer(int id, float dt)
{
    float g[3] = { 0.0f, -9.8f, 0.0f };

    if (time_state() < 2.f)
	game_step(g, dt, 0);
		
    gui_timer(id, dt);
    audio_timer(dt);
}

static void demo_end_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void demo_end_stick(int id, int a, int v)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int demo_end_click(int b, int d)
{
    if (d && b < 0)
        return demo_end_action(gui_token(gui_click()));
    return 1;
}

static int demo_end_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return demo_end_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return demo_end_action(DEMO_KEEP);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int demo_del_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    demo_replay_stop(i == DEMO_DEL);
    return goto_state(&st_demo);
}

static int demo_del_enter(void)
{
    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        kd = gui_label(id, _("Delete Replay?"), GUI_MED, GUI_ALL, gui_red, gui_red);

        if ((jd = gui_harray(id)))
        {
            gui_state(jd, _("Yes"), GUI_SML, DEMO_DEL,  0);
            gui_start(jd, _("No"),  GUI_SML, DEMO_KEEP, 1);
        }

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);
    }
    audio_music_fade_out(2.0f);

    return id;
}

static void demo_del_leave(int id)
{
    gui_delete(id);
}

static void demo_del_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void demo_del_timer(int id, float dt)
{
    gui_timer(id, dt);
    audio_timer(dt);
}

static void demo_del_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void demo_del_stick(int id, int a, int v)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int demo_del_click(int b, int d)
{
    if (d && b < 0)
        return demo_del_action(gui_token(gui_click()));
    return 1;
}

static int demo_del_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return demo_del_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return demo_del_action(DEMO_KEEP);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_demo = {
    demo_enter,
    demo_leave,
    demo_paint,
    demo_timer,
    demo_point,
    demo_stick,
    demo_click,
    NULL,
    demo_buttn,
    0
};

struct state st_demo_play = {
    demo_play_enter,
    demo_play_leave,
    demo_play_paint,
    demo_play_timer,
    NULL,
    NULL,
    NULL,
    NULL,
    demo_play_buttn,
    0
};

struct state st_demo_end = {
    demo_end_enter,
    demo_end_leave,
    demo_end_paint,
    demo_end_timer,
    demo_end_point,
    demo_end_stick,
    demo_end_click,
    NULL,
    demo_end_buttn,
    1, 0
};

struct state st_demo_del = {
    demo_del_enter,
    demo_del_leave,
    demo_del_paint,
    demo_del_timer,
    demo_del_point,
    demo_del_stick,
    demo_del_click,
    NULL,
    demo_del_buttn,
    1, 0
};
