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

#define GOAL_NEXT 2
#define GOAL_SAVE 3

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

    audio_play(AUD_MENU, 1.0f);

    config_get_name(player);
    l = strlen(player);

    switch (i)
    {
    case GOAL_SAVE:
        return goto_state(&st_save);

    case GOAL_NEXT:
        while (level_count())
            ;
        if (level_pass(NULL))
            return goto_state(&st_level);
        else
            return goto_state(&st_title);

    case GUI_CL:
        gui_keyboard_lock();
        break;

    case GUI_BS:
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
            player[l + 0] = gui_keyboard_char((char) i);
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
    const char *s0 = "Set Complete!";
    const char *s1 = "New Record";
    const char *s2 = "GOAL";

    int id, jd, kd;

    level_stat();

    time_i = 3;
    coin_i = 3;
    high   = level_sort(&time_i, &coin_i);

    if ((id = gui_vstack(0)))
    {
        int gid;

        if (level_last())
            gid = gui_label(id, s0, GUI_MED, GUI_ALL, gui_yel, gui_grn);
        else if (high)
            gid = gui_label(id, s1, GUI_MED, GUI_ALL, gui_grn, gui_grn);
        else
            gid = gui_label(id, s2, GUI_LRG, GUI_ALL, gui_blu, gui_grn);

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

        if ((jd = gui_harray(id)))
        {
            gui_state(jd, "Save Replay", GUI_SML, GOAL_SAVE, 0);

            if (level_last())
                gui_start(jd, "Main Menu",  GUI_SML, GOAL_NEXT, 0);
            else
                gui_start(jd, "Next Level", GUI_SML, GOAL_NEXT, 0);
        }

        if (high) gui_keyboard(id);

        gui_layout(id, 0, 0);
        gui_pulse(gid, 1.2f);
    }

    set_most_coins(curr_level(), 3);
    set_best_times(curr_level(), 3);

    audio_music_fade_out(2.0f);
    audio_play(AUD_GOAL, 1.0f);

    config_clr_grab();

    return id;
}

static void goal_leave(int id)
{
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
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void goal_stick(int id, int a, int v)
{
    if (config_tst(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int goal_click(int b, int d)
{
    if (b <= 0 && d == 1)
        return goal_action(gui_token(gui_click()));
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

#define SAVE_SAVE   2
#define SAVE_CANCEL 3

static int  file_id;
static char filename[MAXNAM];

static int save_action(int i)
{
    size_t l = strlen(filename);

    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case SAVE_SAVE:
        if (demo_exists(filename))
            return goto_state(&st_clobber);
        else
        {
            if (level_pass(filename))
                return goto_state(&st_level);
            else
                return goto_state(&st_title);
        }

    case SAVE_CANCEL:
        if (level_pass(NULL))
            return goto_state(&st_level);
        else
            return goto_state(&st_title);

    case GUI_CL:
        gui_keyboard_lock();
        break;

    case GUI_BS:
        if (l > 0)
        {
            filename[l - 1] = 0;
            gui_set_label(file_id, filename);
        }
        break;

    default:
        if (l < MAXNAM - 1)
        {
            filename[l + 0] = gui_keyboard_char((char) i);
            filename[l + 1] = 0;
            gui_set_label(file_id, filename);
        }
    }
    return 1;
}

static int save_enter(void)
{
    int id, jd;

    demo_unique(filename);

    if ((id = gui_vstack(0)))
    {
        gui_pulse(gui_label(id, "Save Replay", GUI_MED, GUI_ALL, 0, 0), 1.2f);

        gui_space(id);
        file_id = gui_label(id, filename, GUI_MED, GUI_ALL, gui_yel, gui_yel);
        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_start(jd, "Cancel", GUI_SML, SAVE_CANCEL, 0);
            gui_start(jd, "Save",   GUI_SML, SAVE_SAVE,   0);
        }
        gui_keyboard(id);
        
        gui_layout(id, 0, 0);
    }

    return id;
}

static void save_leave(int id)
{
    gui_delete(id);
}

static void save_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void save_timer(int id, float dt)
{
    gui_timer(id, dt);
    audio_timer(dt);
}

static void save_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void save_stick(int id, int a, int v)
{
    if (config_tst(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int save_click(int b, int d)
{
    if (b <= 0 && d == 1)
        return save_action(gui_token(gui_click()));
    return 1;
}

static int save_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        goto_state(&st_over);
    return 1;
}

static int save_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return save_click(0, 1);
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int clobber_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    if (i == SAVE_SAVE)
    {
            if (level_pass(filename))
                return goto_state(&st_level);
            else
                return goto_state(&st_title);
    }
    return goto_state(&st_save);
}

static int clobber_enter(void)
{
    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        kd = gui_label(id, "Overwrite?", GUI_MED, GUI_ALL, gui_red, gui_red);

        gui_label(id, filename, GUI_MED, GUI_ALL, gui_yel, gui_yel);

        if ((jd = gui_harray(id)))
        {
            gui_state(jd, "Yes", GUI_SML, SAVE_SAVE,   0);
            gui_start(jd, "No",  GUI_SML, SAVE_CANCEL, 1);
        }

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);
    }

    return id;
}

static void clobber_leave(int id)
{
    gui_delete(id);
}

static void clobber_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void clobber_timer(int id, float dt)
{
    gui_timer(id, dt);
    audio_timer(dt);
}

static int clobber_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? clobber_action(SAVE_CANCEL) : 1;
}

static void clobber_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void clobber_stick(int id, int a, int v)
{
    if (config_tst(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int clobber_click(int b, int d)
{
    if (d && b < 0)
        return clobber_action(gui_token(gui_click()));
    return 1;
}

static int clobber_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst(CONFIG_JOYSTICK_BUTTON_A, b))
            return clobber_action(gui_token(gui_click()));
        if (config_tst(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return clobber_action(SAVE_CANCEL);
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

struct state st_save = {
    save_enter,
    save_leave,
    save_paint,
    save_timer,
    save_point,
    save_stick,
    save_click,
    save_keybd,
    save_buttn,
    1, 0
};

struct state st_clobber = {
    clobber_enter,
    clobber_leave,
    clobber_paint,
    clobber_timer,
    clobber_point,
    clobber_stick,
    clobber_click,
    clobber_keybd,
    clobber_buttn,
    1, 0
};
