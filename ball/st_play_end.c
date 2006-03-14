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

#include "st_play_end.h"
#include "st_save.h"
#include "st_over.h"
#include "st_done.h"
#include "st_start.h"
#include "st_level.h"
#include "st_name.h"

/*---------------------------------------------------------------------------*/

#define PLAY_END_NEXT 2
#define PLAY_END_SAME 3
#define PLAY_END_SAVE 4
#define PLAY_END_BACK 5
#define PLAY_END_DONE 6
#define PLAY_END_NAME 7
#define PLAY_END_OVER 8

static int balls_id;
static int coins_id;
static int score_id;

extern struct state st_play_end_bis;

static int play_end_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case PLAY_END_BACK:
    case PLAY_END_OVER:
	return goto_end_level();

    case PLAY_END_SAVE:
        return goto_save(&st_play_end_bis, &st_play_end_bis);

    case PLAY_END_NAME:
        return goto_name(&st_play_end_bis, &st_play_end_bis);
	
    case PLAY_END_DONE:
	return goto_state(&st_done);
	
    case PLAY_END_NEXT:
	level_next();
	return goto_state(&st_level);

    case PLAY_END_SAME:
	return goto_state(&st_level);
    }

    return 1;
}

static int play_end_init(int * gidp)
{
    const struct level_game *lg = curr_lg();
    int mode  = lg->mode;
    int state = lg->state;
    const struct level *l = lg->level;

    int id, jd, kd;
    int high;

    high = (lg->time_rank < 3) || (lg->goal_rank < 3) || (lg->coin_rank < 3);

    if ((id = gui_vstack(0)))
    {
        int gid;

	if      (state == GAME_SPEC)
	    gid = gui_label(id, _("Special Goal"), GUI_MED, GUI_ALL, gui_yel, gui_yel);
	else if (state == GAME_FALL)
	    gid = gui_label(id, _("Fall-out!"),    GUI_MED, GUI_ALL, gui_yel, gui_yel);
	else if (state == GAME_TIME)
	    gid = gui_label(id, _("Time's Up!"),   GUI_MED, GUI_ALL, gui_yel, gui_yel);
	else if (high)
            gid = gui_label(id, _("New Record"),   GUI_MED, GUI_ALL, gui_grn, gui_grn);
        else
            gid = gui_label(id, _("GOAL"),         GUI_LRG, GUI_ALL, gui_blu, gui_grn);

        gui_space(id);

	if (mode == MODE_CHALLENGE && 
		(lg->state == GAME_GOAL || lg->state == GAME_SPEC || l->is_bonus))
	{
	    int coins = lg->coins;
	    int score = lg->score - coins;
	    int balls = lg->balls - count_extra_balls(score, coins);
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
	else
	{
	    balls_id = score_id = coins_id = 0;
	}
	
	if (state == GAME_GOAL)
	{
	    gui_space(id);
	    if ((jd = gui_harray(id)))
	    {
                gui_most_coins(jd, 1);
		gui_best_times(jd, 1);
	    }
        }

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
	    int nlid = 0, rlid = 0;
	    int b = 0;
	    if (lg->win)
                gui_start(jd, _("Finish"),      GUI_SML, PLAY_END_DONE, 0);
	    else
                nlid = gui_maybe(jd, _("Next Level"),  PLAY_END_NEXT, lg->next_level != NULL);
	    
	    if (lg->dead)
		gui_start(jd, _("Game Over"), GUI_SML, PLAY_END_OVER, 0);
	    else
	    {
		b = mode != MODE_CHALLENGE || 
		    ((state == GAME_FALL || state == GAME_TIME) && !lg->dead && !l->is_bonus);
		rlid = gui_maybe(jd, _("Retry Level"), PLAY_END_SAME, b);
	    }
	    
	    gui_maybe(jd, _("Save Replay"), PLAY_END_SAVE, demo_play_saved());

	    /* default is next if the next level is newly unkocked */
	    if (nlid != 0 && lg->unlock)
		 gui_focus(nlid);
	    else if(b)
		 gui_focus(rlid);
        }

        if (high)
	    gui_state(id, _("Change Player Name"),  GUI_SML, PLAY_END_NAME, 0);

        gui_layout(id, 0, 0);
	if (gidp) *gidp = gid;
    }

    if (state == GAME_GOAL)
    {
	set_most_coins(&l->coin_score, lg->coin_rank);
	if (mode == MODE_CHALLENGE || mode == MODE_NORMAL)
	    set_best_times(&l->goal_score, lg->goal_rank, 1);
	else
	    set_best_times(&l->time_score, lg->time_rank, 0);
    }

    config_clr_grab();

    return id;
}

static int play_end_enter(void)
{
    int gid;
    int r;
    
    r = play_end_init(&gid);
    
    gui_pulse(gid, 1.2f);
    audio_music_fade_out(2.0f);
    return r; 
}

static int play_end_bis_enter(void)
{
    level_update_player_name();
    return play_end_init(NULL);
}

static void play_end_timer(int id, float dt)
{
    static float DT = 0.0f;

    float gg[3] = { 0.0f, 9.8f, 0.0f };
    float gf[3] = { 0.0f, -9.8f, 0.0f };
    int state = curr_lg()->state;

    DT += dt;

    if (time_state() < 2.f)
        game_step((state == GAME_GOAL || state == GAME_SPEC) ? gg : gf, dt, NULL);
    
    if (time_state() > 1.f && DT > 0.05f && coins_id != 0)
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

static int play_end_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return play_end_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
	    return play_end_action(PLAY_END_BACK);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_play_end = {
    play_end_enter,
    shared_leave,
    shared_paint,
    play_end_timer,
    shared_point,
    shared_stick,
    shared_click,
    NULL,
    play_end_buttn,
    1, 0
};

struct state st_play_end_bis = {
    play_end_bis_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_click,
    NULL,
    play_end_buttn,
    1, 0
};
