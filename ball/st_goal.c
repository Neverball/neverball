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
#include "transition.h"
#include "util.h"
#include "progress.h"
#include "audio.h"
#include "config.h"
#include "video.h"
#include "demo.h"
#include "hud.h"
#include "key.h"

#include "game_common.h"
#include "game_server.h"
#include "game_client.h"

#include "st_goal.h"
#include "st_save.h"
#include "st_level.h"
#include "st_name.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

enum
{
    GOAL_NEXT = GUI_LAST,
    GOAL_SAME,
    GOAL_SAVE,
    GOAL_DONE,
    GOAL_LAST
};

static int balls_id;
static int coins_id;
static int score_id;

static int resume;

static int goal_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case GUI_BACK:
    case GOAL_LAST:
        progress_stop();
        return goto_exit();

    case GOAL_SAVE:
        progress_stop();
        return goto_save(&st_goal, &st_goal);

    case GUI_NAME:
        progress_stop();
        return goto_name(&st_goal, &st_goal, 0);

    case GOAL_DONE:
        progress_stop();
        progress_exit();
        return goto_exit();

    case GUI_SCORE:
        gui_score_set(val);
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

static int goal_gui(void)
{
    const char *s1 = _("New Record");
    const char *s2 = _("GOAL");

    int id, jd, kd, ld, md;
    int root_id;

    int high = progress_lvl_high();

    if ((root_id = gui_root()))
    {
        if ((id = gui_vstack(root_id)))
        {
            int gid;

            if (high)
                gid = gui_label(id, s1, GUI_MED, gui_grn, gui_grn);
            else
                gid = gui_label(id, s2, GUI_MED, gui_blu, gui_grn);

            gui_space(id);

            if (curr_mode() == MODE_CHALLENGE)
            {
                int coins, score, balls;
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

                if ((jd = gui_hstack(id)))
                {
                    gui_filler(jd);

                    if ((kd = gui_vstack(jd)))
                    {
                        if ((ld = gui_hstack(kd)))
                        {
                            if ((md = gui_harray(ld)))
                            {
                                balls_id = gui_count(md, 100, GUI_MED);
                                gui_label(md, _("Balls"), GUI_SML,
                                        gui_wht, gui_wht);
                            }
                            if ((md = gui_harray(ld)))
                            {
                                score_id = gui_count(md, 1000, GUI_MED);
                                gui_label(md, _("Score"), GUI_SML,
                                        gui_wht, gui_wht);
                            }
                            if ((md = gui_harray(ld)))
                            {
                                coins_id = gui_count(md, 100, GUI_MED);
                                gui_label(md, _("Coins"), GUI_SML,
                                        gui_wht, gui_wht);
                            }

                            gui_set_count(balls_id, balls);
                            gui_set_count(score_id, score);
                            gui_set_count(coins_id, coins);
                        }

                        if ((ld = gui_harray(kd)))
                        {
                            const struct level *l;

                            gui_label(ld, "", GUI_SML, 0, 0);

                            for (i = MAXLVL - 1; i >= 0; i--)
                                if ((l = get_level(i)) && level_bonus(l))
                                {
                                    const GLubyte *c = (level_opened(l) ?
                                                        gui_grn : gui_gry);

                                    gui_label(ld, level_name(l), GUI_SML, c, c);
                                }

                            gui_label(ld, "", GUI_SML, 0, 0);
                        }

                        gui_set_rect(kd, GUI_ALL);
                    }

                    gui_filler(jd);
                }

                gui_space(id);
            }
            else
            {
                balls_id = score_id = coins_id = 0;
            }

            gui_score_board(id, (GUI_SCORE_COIN |
                                GUI_SCORE_TIME |
                                GUI_SCORE_GOAL), 1, high);

            gui_space(id);

            if ((jd = gui_harray(id)))
            {
                if      (progress_done())
                    gui_start(jd, _("Finish"), GUI_SML, GOAL_DONE, 0);
                else if (progress_last())
                    gui_start(jd, _("Finish"), GUI_SML, GOAL_LAST, 0);

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

        if ((id = gui_vstack(root_id)))
        {
            gui_space(id);

            if ((jd = gui_hstack(id)))
            {
                gui_back_button(jd);
                gui_space(jd);
            }

            gui_layout(id, -1, +1);
        }
    }

    gui_set_stats(curr_level());

    set_score_board(level_score(curr_level(), SCORE_COIN), progress_coin_rank(),
                    level_score(curr_level(), SCORE_TIME), progress_time_rank(),
                    level_score(curr_level(), SCORE_GOAL), progress_goal_rank());

    return root_id;
}

static int goal_enter(struct state *st, struct state *prev, int intent)
{
    if (prev == &st_name)
        progress_rename(0);

    audio_music_fade_out(2.0f);
    video_clr_grab();
    resume = (prev == &st_goal || prev == &st_name || prev == &st_save);
    return transition_slide(goal_gui(), 1, intent);
}

static void goal_paint(int id, float t)
{
    game_client_draw(0, t);
    gui_paint(id);
    hud_paint();
}

static void goal_timer(int id, float dt)
{
    if (!resume)
    {
        static float t = 0.0f;

        t += dt;

        if (time_state() < 1.f)
        {
            game_server_step(dt);
            game_client_sync(demo_fp);
            game_client_blend(game_server_blend());
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
    }

    gui_timer(id, dt);
    hud_timer(dt);
}

static int goal_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
            return goal_action(GUI_BACK, 0);
        if (config_tst_d(CONFIG_KEY_SCORE_NEXT, c))
            return goal_action(GUI_SCORE, GUI_SCORE_NEXT(gui_score_get()));
        if (config_tst_d(CONFIG_KEY_RESTART, c) && progress_same_avail())
            return goal_action(GOAL_SAME, 0);
    }

    return 1;
}

static int goal_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goal_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return goal_action(GUI_BACK, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_goal = {
    goal_enter,
    shared_leave,
    goal_paint,
    goal_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    goal_keybd,
    goal_buttn
};

