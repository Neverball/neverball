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
#include "util.h"
#include "demo.h"
#include "level.h"
#include "audio.h"
#include "config.h"

#include "st_goal.h"
#include "st_high.h"
#include "st_over.h"
#include "st_title.h"
#include "st_level.h"

/*---------------------------------------------------------------------------*/

static int goal_enter(void)
{
    int id, jd;

    if ((id = gui_vstack(0)))
    {
        gui_pulse(gui_label(id, "GOAL", GUI_LRG, GUI_ALL, gui_blu, gui_grn), 1.2f);
        gui_space(id);

        gui_curr_score(id, curr_level());
        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_most_coins(jd);
            gui_best_times(jd);
        }

        gui_layout(id, 0, 0);
    }

    set_most_coins(curr_level());
    set_best_times(curr_level());

    audio_music_fade(3.f);
    audio_play(AUD_GOAL, 1.f);

    return id;
}

static void goal_leave(int id)
{
    demo_finish();
    gui_delete(id);
    audio_music_play("bgm/inter.ogg");
}

static void goal_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void goal_timer(int id, float dt)
{
    float g[3] = { 0.0f, 9.8f, 0.0f };

    if (time_state() < 1.f)
    {
        game_step(g, dt, 0);
        demo_play_step(dt);
    }

    gui_timer(id, dt);
}

static int goal_click(int b, int d)
{
    if (b <= 0 && d == 1)
    {
        if (level_goal())
            goto_state(&st_high);
        else
        {
            if (level_pass())
                goto_state(&st_level);
            else
                goto_state(&st_done);
        }
    }
    return 1;
}

static int goal_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        goto_state(&st_over);
    return 1;
}

static int goal_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return goal_click(0, 1);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int done_enter(void)
{
    const char *s0 = "Congrats!";
    const char *s1 = "That's it for this set.";
    const char *s2 = "Move on to the next one.";
    int id, jd;

    if ((id = gui_vstack(0)))
    {
        gui_pulse(gui_label(id, s0, GUI_LRG, GUI_ALL, gui_grn, gui_yel), 1.2f);
        gui_space(id);

        if ((jd = gui_varray(id)))
        {
            gui_label(jd, s1, GUI_SML, GUI_TOP, gui_wht, gui_wht);
            gui_label(jd, s2, GUI_SML, GUI_BOT, gui_wht, gui_wht);
        }

        gui_layout(id, 0, 0);
    }

    audio_play(AUD_OVER, 1.f);

    return id;
}

static void done_leave(int id)
{
    gui_delete(id);
    set_free();
}

static void done_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void done_timer(int id, float dt)
{
    if (dt > 0.f && time_state() > 10.f)
        goto_state(&st_title);

    gui_timer(id, dt);
}

static int done_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int done_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_title) : 1;
}

static int done_buttn(int b, int d)
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

struct state st_goal = {
    goal_enter,
    goal_leave,
    goal_paint,
    goal_timer,
    NULL,
    NULL,
    goal_click,
    goal_keybd,
    goal_buttn,
    1, 0
};

struct state st_done = {
    done_enter,
    done_leave,
    done_paint,
    done_timer,
    NULL,
    NULL,
    done_click,
    done_keybd,
    done_buttn,
    1, 0
};
