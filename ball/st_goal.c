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

#include <string.h>

#include "gui.h"
#include "set.h"
#include "game.h"
#include "util.h"
#include "demo.h"
#include "level.h"
#include "audio.h"
#include "config.h"

#include "st_goal.h"
#include "st_over.h"
#include "st_title.h"
#include "st_level.h"

/*---------------------------------------------------------------------------*/

#define HIGH_OK 0
#define HIGH_BS 1

static int high;
static int time_i;
static int coin_i;
    
static int balls_id;
static int coins_id;
static int score_id;

static int goal_action(int i)
{
    char player[MAXNAM];
    size_t l;

    config_get_name(player);
    l = strlen(player);

    switch (i)
    {
    case HIGH_OK:
        while (level_count())
            ;
        if (level_pass())
            return goto_state(&st_level);
        else
            return goto_state(&st_done);

    case HIGH_BS:
        if (l > 0)
        {
            player[l - 1] = 0;

            config_set_name(player);
            level_name(player, time_i, coin_i);
            set_most_coins(curr_level(), 3);
            set_best_times(curr_level(), 3);
        }
        break;

    default:
        if (l < MAXNAM - 1)
        {
            player[l + 0] = (char) i;
            player[l + 1] = 0;

            config_set_name(player);
            level_name(player, time_i, coin_i);
            set_most_coins(curr_level(), 3);
            set_best_times(curr_level(), 3);
        }
    }
    return 1;
}

static int goal_enter(void)
{
    int id, jd, kd, ld;

    time_i = 3;
    coin_i = 3;
    high   = level_sort(&time_i, &coin_i);

    if ((id = gui_vstack(0)))
    {
        int gid = gui_label(id, "GOAL", GUI_LRG, GUI_ALL, gui_blu, gui_grn);

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            if ((kd = gui_harray(jd)))
            {
                balls_id = gui_count(kd, curr_balls(), GUI_MED, GUI_RGT);
                gui_label(kd, "Balls", GUI_SML, GUI_LFT, gui_wht, gui_wht);
            }
            if ((kd = gui_harray(jd)))
            {
                score_id = gui_count(kd, curr_score(), GUI_MED, GUI_RGT);
                gui_label(kd, "Score", GUI_SML, GUI_LFT, gui_wht, gui_wht);
            }
            if ((kd = gui_harray(jd)))
            {
                coins_id = gui_count(kd, curr_coins(), GUI_MED, GUI_RGT);
                gui_label(kd, "Coins", GUI_SML, GUI_LFT, gui_wht, gui_wht);
            }
        }

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_most_coins(jd, 3, coin_i);
            gui_best_times(jd, 3, time_i);
        }

        gui_space(id);

        if (high)
        {
            if ((jd = gui_hstack(id)))
            {
                gui_filler(jd);

                if ((kd = gui_varray(jd)))
                {
                    if ((ld = gui_harray(kd)))
                    {
                        gui_state(ld, "G", GUI_SML, 'G', 0);
                        gui_state(ld, "F", GUI_SML, 'F', 0);
                        gui_state(ld, "E", GUI_SML, 'E', 0);
                        gui_state(ld, "D", GUI_SML, 'D', 0);
                        gui_state(ld, "C", GUI_SML, 'C', 0);
                        gui_state(ld, "B", GUI_SML, 'B', 0);
                        gui_state(ld, "A", GUI_SML, 'A', 0);
                    }
                    if ((ld = gui_harray(kd)))
                    {
                        gui_state(ld, "N", GUI_SML, 'N', 0);
                        gui_state(ld, "M", GUI_SML, 'M', 0);
                        gui_state(ld, "L", GUI_SML, 'L', 0);
                        gui_state(ld, "K", GUI_SML, 'K', 0);
                        gui_state(ld, "J", GUI_SML, 'J', 0);
                        gui_state(ld, "I", GUI_SML, 'I', 0);
                        gui_state(ld, "H", GUI_SML, 'H', 0);
                    }
                    if ((ld = gui_harray(kd)))
                    {
                        gui_state(ld, "U", GUI_SML, 'U', 0);
                        gui_state(ld, "T", GUI_SML, 'T', 0);
                        gui_state(ld, "S", GUI_SML, 'S', 0);
                        gui_state(ld, "R", GUI_SML, 'R', 0);
                        gui_state(ld, "Q", GUI_SML, 'Q', 0);
                        gui_state(ld, "P", GUI_SML, 'P', 0);
                        gui_state(ld, "O", GUI_SML, 'O', 0);
                    }
                    if ((ld = gui_harray(kd)))
                    {
                        gui_start(ld, "Ok", GUI_SML, HIGH_OK, 0);
                        gui_state(ld, "<",  GUI_SML, HIGH_BS, 0);

                        gui_state(ld, "Z", GUI_SML, 'Z', 0);
                        gui_state(ld, "Y", GUI_SML, 'Y', 0);
                        gui_state(ld, "X", GUI_SML, 'X', 0);
                        gui_state(ld, "W", GUI_SML, 'W', 0);
                        gui_state(ld, "V", GUI_SML, 'V', 0);
                    }
                }

                gui_filler(jd);
            }
        }

        gui_layout(id, 0, 0);
        gui_pulse(gid, 1.2f);
    }

    set_most_coins(curr_level(), 3);
    set_best_times(curr_level(), 3);

    audio_music_fade_out(2.0f);
    audio_play(AUD_GOAL, 1.0f);

    return id;
}

static void goal_leave(int id)
{
    demo_finish();
    gui_delete(id);
}

static void goal_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void goal_timer(int id, float dt)
{
    static float DT = 0.0f;

    float g[3] = { 0.0f, 9.8f, 0.0f };

    DT += dt;

    if (time_state() < 1.f)
    {
        game_step(g, dt, 0);
        demo_play_step(dt);
    }
    else if (DT > 0.05f)
    {
        if (level_count())
        {
            int coins = curr_coins();
            int score = curr_score();
            int balls = curr_balls();

            if (gui_value(coins_id) != coins)
            {
                gui_set_count(coins_id, coins);
                gui_pulse(coins_id, 1.1f);
            }
            if (gui_value(score_id) != score)
            {
                gui_set_count(score_id, score);
                gui_pulse(score_id, 1.1f);
            }
            if (gui_value(balls_id) != balls)
            {
                gui_set_count(balls_id, balls);
                gui_pulse(balls_id, 2.0f);
            }
        }

        DT = 0.0f;
    }

    gui_timer(id, dt);
    audio_timer(dt);
}

static void goal_point(int id, int x, int y, int dx, int dy)
{
    if (high)
    {
        gui_pulse(gui_point(id, x, y), 1.2f);
    }
}

static void goal_stick(int id, int a, int v)
{
    if (high)
    {
        if (config_tst(CONFIG_JOYSTICK_AXIS_X, a))
            gui_pulse(gui_stick(id, v, 0), 1.2f);
        if (config_tst(CONFIG_JOYSTICK_AXIS_Y, a))
            gui_pulse(gui_stick(id, 0, v), 1.2f);
    }
}

static int goal_click(int b, int d)
{
    if (b <= 0 && d == 1)
        return goal_action(high ? gui_token(gui_click()) : HIGH_OK);
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
    audio_timer(dt);
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
    goal_point,
    goal_stick,
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
