/*
 * Copyright (C) 2007 Robert Kooima
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

#include <stdio.h>

#include "gui.h"
#include "game.h"
#include "util.h"
#include "progress.h"
#include "audio.h"
#include "config.h"
#include "demo.h"

#include "st_goal.h"
#include "st_save.h"
#include "st_over.h"
#include "st_done.h"
#include "st_start.h"
#include "st_level.h"
#include "st_name.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

#define GOAL_NEXT 1
#define GOAL_SAME 2
#define GOAL_SAVE 3
#define GOAL_BACK 4
#define GOAL_DONE 5
#define GOAL_OVER 6

static int balls_id;
static int coins_id;
static int score_id;

/* Bread crumbs. */

static int new_name;
static int resume;

static int goal_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case GOAL_BACK:
        /* Fall through. */

    case GOAL_OVER:
        progress_stop();
        return goto_state(&st_over);

    case GOAL_SAVE:
        resume = 1;

        progress_stop();
        return goto_save(&st_goal, &st_goal);

    case GUI_NAME:
        new_name = 1;
        resume = 1;

        progress_stop();
        return goto_name(&st_goal, &st_goal, 0);

    case GOAL_DONE:
        progress_stop();
        progress_exit();
        return goto_state(&st_done);

    case GUI_MOST_COINS:
    case GUI_BEST_TIMES:
    case GUI_UNLOCK_GOAL:
        gui_score_set(i);
        resume = 1;
        return goto_state(&st_goal);

    case GOAL_NEXT:
        if (progress_next())
            return goto_state(&st_level);
        break;

    case GOAL_SAME:
        if (progress_same())
            return goto_state(&st_level);
        break;
    }

    return 1;
}

static int goal_enter(void)
{
    const char *s1 = _("New Record");
    const char *s2 = _("GOAL");

    int id, jd, kd;

    const struct level *l = get_level(curr_level());

    int high = progress_lvl_high();

    if (new_name)
    {
        progress_rename(0);
        new_name = 0;
    }

    if ((id = gui_vstack(0)))
    {
        int gid;

        if (high)
            gid = gui_label(id, s1, GUI_MED, GUI_ALL, gui_grn, gui_grn);
        else
            gid = gui_label(id, s2, GUI_LRG, GUI_ALL, gui_blu, gui_grn);

        gui_space(id);

        if (curr_mode() == MODE_CHALLENGE)
        {
            int coins, score, balls;
            char msg[MAXSTR] = "";
            int i;

            /* Reverse-engineer initial score and balls. */

            if (resume)
            {
                coins = 0;
                score = curr_score();
                balls = curr_balls();
            }
            else
            {
                coins = curr_coins();
                score = curr_score() - coins;
                balls = curr_balls();

                for (i = curr_score(); i > score; i--)
                    if (progress_reward_ball(i))
                        balls--;
            }

            sprintf(msg, ngettext("%d new bonus level",
                                  "%d new bonus levels",
                                  curr_bonus()), curr_bonus());

            if ((jd = gui_hstack(id)))
            {

                if ((kd = gui_harray(jd)))
                {
                    balls_id = gui_count(kd, 100, GUI_MED, GUI_NE);
                    gui_label(kd, _("Balls"), GUI_SML, 0, gui_wht, gui_wht);
                }
                if ((kd = gui_harray(jd)))
                {
                    score_id = gui_count(kd, 1000, GUI_MED, 0);
                    gui_label(kd, _("Score"), GUI_SML, 0, gui_wht, gui_wht);
                }
                if ((kd = gui_harray(jd)))
                {
                    coins_id = gui_count(kd, 100, GUI_MED, 0);
                    gui_label(kd, _("Coins"), GUI_SML, GUI_NW, gui_wht, gui_wht);
                }

                gui_set_count(balls_id, balls);
                gui_set_count(score_id, score);
                gui_set_count(coins_id, coins);

            }

            gui_label(id, msg, GUI_SML, GUI_BOT, 0, 0);

            gui_space(id);
        }
        else
        {
            balls_id = score_id = coins_id = 0;
        }

        if ((jd = gui_hstack(id)))
            gui_score_board(jd, GUI_MOST_COINS |
                                GUI_BEST_TIMES |
                                GUI_UNLOCK_GOAL, 1, high);

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            if (progress_done())
                gui_start(jd, _("Finish"), GUI_SML, GOAL_DONE, 0);

            if (progress_next_avail())
                gui_start(jd, _("Next Level"),  GUI_SML, GOAL_NEXT, 0);

            if (progress_same_avail())
                gui_start(jd, _("Retry Level"), GUI_SML, GOAL_SAME, 0);

            if (demo_saved())
                gui_state(jd, _("Save Replay"), GUI_SML, GOAL_SAVE, 0);
        }

        if (!resume)
            gui_pulse(gid, 1.2f);

        gui_layout(id, 0, 0);

    }

    set_score_board(&l->score.most_coins,  progress_coin_rank(),
                    &l->score.best_times,  progress_time_rank(),
                    &l->score.unlock_goal, progress_goal_rank());

    audio_music_fade_out(2.0f);

    config_clr_grab();

    /* Reset hack. */
    resume = 0;

    return id;
}

static void goal_timer(int id, float dt)
{
    static float t = 0.0f;

    float g[3] = { 0.0f, 9.8f, 0.0f };

    t += dt;

    if (time_state() < 1.f)
    {
        demo_play_step();
        game_step(g, dt, 0);
    }
    else if (t > 0.05f && coins_id)
    {
        int coins = gui_value(coins_id);

        if (coins > 0)
        {
            int score = gui_value(score_id);
            int balls = gui_value(balls_id);

            gui_set_count(coins_id, coins - 1);
            gui_pulse(coins_id, 1.1f);

            gui_set_count(score_id, score + 1);
            gui_pulse(score_id, 1.1f);

            if (progress_reward_ball(score + 1))
            {
                gui_set_count(balls_id, balls + 1);
                gui_pulse(balls_id, 2.0f);
                audio_play(AUD_BALL, 1.0f);
            }
        }
        t = 0.0f;
    }

    gui_timer(id, dt);
}

static int goal_keybd(int c, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_KEY_SCORE_NEXT, c))
            return goal_action(gui_score_next(gui_score_get()));
        if (config_tst_d(CONFIG_KEY_RESTART, c) && progress_same_avail())
            return goal_action(GOAL_SAME);
    }

    return 1;
}

static int goal_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goal_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goal_action(GOAL_BACK);
    }
    return 1;
}

static void goal_leave(int id)
{
    /* HACK:  don't run animation if only "visiting" a state. */
    st_goal.timer = resume ? shared_timer : goal_timer;

    gui_delete(id);
}

/*---------------------------------------------------------------------------*/

struct state st_goal = {
    goal_enter,
    goal_leave,
    shared_paint,
    goal_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    goal_keybd,
    goal_buttn,
    1, 0
};

