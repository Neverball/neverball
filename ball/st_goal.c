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
#include "levels.h"
#include "audio.h"
#include "config.h"
#include "demo.h"
#include "st_shared.h"

#include "st_goal.h"
#include "st_save.h"
#include "st_over.h"
#include "st_done.h"
#include "st_start.h"
#include "st_level.h"
#include "st_name.h"

/*---------------------------------------------------------------------------*/

#define GOAL_NEXT 2
#define GOAL_SAME 3
#define GOAL_SAVE 4
#define GOAL_BACK 5
#define GOAL_DONE 6
#define GOAL_NAME 7

static int high;
static int time_i;
static int coin_i;
    
static int balls_id;
static int coins_id;
static int score_id;

extern struct state st_goal_bis;

static int goal_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case GOAL_BACK:
	return goto_end_level();

    case GOAL_SAVE:
        return goto_save(&st_goal_bis, &st_goal_bis);

    case GOAL_NAME:
        return goto_name(&st_goal_bis, &st_goal_bis);
	
    case GOAL_DONE:
	return goto_state(&st_done);
	
    case GOAL_NEXT:
	level_next();
	return goto_state(&st_level);

    case GOAL_SAME:
	return goto_state(&st_level);
    }

    return 1;
}

static int goal_init(int * gidp)
{
    const char *s1 = _("New Record");
    const char *s2 = _("GOAL");

    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        int gid;

        if (high)
            gid = gui_label(id, s1, GUI_MED, GUI_ALL, gui_grn, gui_grn);
        else
            gid = gui_label(id, s2, GUI_LRG, GUI_ALL, gui_blu, gui_grn);

        gui_space(id);

	if (level_mode() == MODE_CHALLENGE)
	{
	    int coins = curr_coins();
	    int score = curr_score() - coins;
	    int balls = curr_balls() - count_extra_balls(score, coins);
	    if ((jd = gui_hstack(id)))
	    {
                if ((kd = gui_harray(jd)))
                {
                    balls_id = gui_count(kd,  100, GUI_MED, GUI_RGT);
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
	
	gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_most_coins(jd, 3, 1);
            gui_best_times(jd, 3, 1);
        }

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
	    if (demo_play_saved())
                gui_state(jd, _("Save Replay"), GUI_SML, GOAL_SAVE, 0);
	    else
                gui_label(jd, _("Save Replay"), GUI_SML, GUI_ALL, gui_blk, gui_blk);
	    
	    if (level_mode() != MODE_CHALLENGE)
                gui_start(jd, _("Retry Level"), GUI_SML, GOAL_SAME, 0);
	    
	    if (level_mode() == MODE_CHALLENGE && level_last())
                gui_start(jd, _("Finish"),      GUI_SML, GOAL_DONE, 0);
	    else if (level_opened(curr_level()+1))
                gui_state(jd, _("Next Level"),  GUI_SML, GOAL_NEXT, 0);
            else if (level_mode() != MODE_SINGLE)
                gui_label(jd, _("Next Level"),  GUI_SML, GUI_ALL, gui_blk, gui_blk);
        }

        if (high)
	    gui_state(id, _("Change Player Name"),  GUI_SML, GOAL_NAME, 0);

        gui_layout(id, 0, 0);
	if (gidp) *gidp = gid;
    }

    set_most_coins(curr_level(), coin_i);
    set_best_times(curr_level(), time_i);

    config_clr_grab();

    return id;
}

static int goal_enter(void)
{
    int gid;
    int r;
    
    time_i = 3;
    coin_i = 3;
    high   = level_sort(&time_i, &coin_i);

    r = goal_init(&gid);
    
    gui_pulse(gid, 1.2f);
    audio_music_fade_out(2.0f);
    return r; 
}

static int goal_bis_enter(void)
{
    char player[MAXNAM];
    config_get_s(CONFIG_PLAYER, player, MAXNAM);
    level_name(curr_level(), player, time_i, coin_i);
    return goal_init(NULL);
}

static void goal_timer(int id, float dt)
{
    static float DT = 0.0f;

    float g[3] = { 0.0f, 9.8f, 0.0f };

    DT += dt;

    if (time_state() < 1.f)
        game_step(g, dt, 0);
    else if (DT > 0.05f && level_mode() == MODE_CHALLENGE)
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
	    if ((score+1) % 100 == 0)
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

/*---------------------------------------------------------------------------*/

struct state st_goal = {
    goal_enter,
    shared_leave,
    shared_paint,
    goal_timer,
    shared_point,
    shared_stick,
    shared_click,
    NULL,
    goal_buttn,
    1, 0
};

struct state st_goal_bis = {
    goal_bis_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_click,
    NULL,
    goal_buttn,
    1, 0
};
