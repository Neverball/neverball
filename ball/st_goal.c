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

#include "gui.h"
#include "game.h"
#include "util.h"
#include "levels.h"
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
#define GOAL_NAME 6
#define GOAL_OVER 7

static int balls_id;
static int coins_id;
static int score_id;

static int new_name;
static int be_back_soon;

static int goal_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case GOAL_BACK:
        /* Fall through. */

    case GOAL_OVER:
        level_stop();
        return goto_state(&st_over);

    case GOAL_SAVE:
        be_back_soon = 1;

        level_stop();
        return goto_save(&st_goal, &st_goal);

    case GOAL_NAME:
        new_name = 1;
        be_back_soon = 1;

        level_stop();
        return goto_name(&st_goal, &st_goal);

    case GOAL_DONE:
        level_stop();
        return goto_state(&st_done);

    case GOAL_NEXT:
        level_next();
        return goto_state(&st_level);

    case GOAL_SAME:
        level_same();
        return goto_state(&st_level);
    }

    return 1;
}

static int goal_enter(void)
{
    int id, jd, kd;

    const struct level_game *lg = curr_lg();
    const struct level *l = lg->level;

    int high;

    high = (lg->time_rank < 3) || (lg->goal_rank < 3) || (lg->coin_rank < 3);

    /* Reset hack. */
    be_back_soon = 0;

    if (new_name)
    {
        level_update_player_name();
        new_name = 0;
    }

    if ((id = gui_vstack(0)))
    {
        int gid;

        if (high)
            gid = gui_label(id, _("New Record"), GUI_MED, GUI_ALL, gui_grn, gui_grn);
        else
            gid = gui_label(id, _("GOAL"), GUI_LRG, GUI_ALL, gui_blu, gui_grn);

        gui_space(id);

        if (lg->mode == MODE_CHALLENGE)
        {
            int coins = lg->coins;
            int score = lg->score - coins;
            int balls = lg->balls - count_extra_balls(score, coins);

            if ((jd = gui_hstack(id)))
            {
                if ((kd = gui_harray(jd)))
                {
                    balls_id = gui_count(kd, 100, GUI_MED, GUI_RGT);
                    gui_label(kd, _("Balls"), GUI_SML, GUI_LFT, gui_wht, gui_wht);
                }
                if ((kd = gui_harray(jd)))
                {
                    score_id = gui_count(kd, 1000, GUI_MED, GUI_RGT);
                    gui_label(kd, _("Score"), GUI_SML, GUI_LFT, gui_wht, gui_wht);
                }
                if ((kd = gui_harray(jd)))
                {
                    coins_id = gui_count(kd, 100, GUI_MED, GUI_RGT);
                    gui_label(kd, _("Coins"), GUI_SML, GUI_LFT, gui_wht, gui_wht);
                }

                gui_set_count(balls_id, balls);
                gui_set_count(score_id, score);
                gui_set_count(coins_id, coins);
            }
        }
        else
        {
            balls_id = score_id = coins_id = 0;
        }

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_most_coins(jd, 1);
            gui_best_times(jd, 1);
        }

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            int next_id = 0, retry_id = 0;

            if (lg->win)
                gui_start(jd, _("Finish"), GUI_SML, GOAL_DONE, 0);
            else
                next_id = gui_maybe(jd, _("Next Level"),  GOAL_NEXT,
                                    lg->next_level != NULL);

            if (lg->dead)
                gui_start(jd, _("Game Over"), GUI_SML, GOAL_OVER, 0);
            else
            {
                retry_id = gui_maybe(jd, _("Retry Level"), GOAL_SAME,
                                     lg->mode != MODE_CHALLENGE);
            }

            gui_maybe(jd, _("Save Replay"), GOAL_SAVE, demo_saved());

            /* Default is next if the next level is newly unlocked. */

            if (next_id && lg->unlock)
                gui_focus(next_id);
            else if (lg->mode != MODE_CHALLENGE)
                gui_focus(retry_id);
        }

        /* FIXME, I'm ugly. */

        if (high)
            gui_state(id, _("Change Player Name"),  GUI_SML, GOAL_NAME, 0);

        gui_pulse(gid, 1.2f);
        gui_layout(id, 0, 0);

    }

    set_most_coins(&l->score.most_coins, lg->coin_rank);

    if (lg->mode == MODE_CHALLENGE || lg->mode == MODE_NORMAL)
        set_best_times(&l->score.unlock_goal, lg->goal_rank, 1);
    else
        set_best_times(&l->score.best_times, lg->time_rank, 0);

    audio_music_fade_out(2.0f);

    config_clr_grab();

    return id;
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
    else if (DT > 0.05f && coins_id)
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

            if ((score + 1) % 100 == 0)
            {
                gui_set_count(balls_id, balls + 1);
                gui_pulse(balls_id, 2.0f);
                audio_play(AUD_BALL, 1.0f);
            }
        }
        DT = 0.0f;
    }

    gui_timer(id, dt);
    audio_timer(dt);
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
    st_goal.timer = be_back_soon ? shared_timer : goal_timer;

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
    shared_click,
    NULL,
    goal_buttn,
    1, 0
};

