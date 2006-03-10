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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "level.h"
#include "levels.h"
#include "game.h"
#include "demo.h"
#include "audio.h"
#include "config.h"
#include "set.h"

/*---------------------------------------------------------------------------*/

/* The level currently playing */
static struct level_game current_level_game;

/*---------------------------------------------------------------------------*/

int level_replay(const char *filename)
{
    return demo_replay_init(filename, &current_level_game); 
}

static struct level single_level; /* a level without set */

int level_play_go(void)
/* Start to play the current level */
{
    struct level_game *lg = &current_level_game;
    const struct level *l = lg->level;
    int mode = lg->mode;

    assert(l != NULL);

    lg->goal = (mode == MODE_PRACTICE) ? 0 : l->goal;
    lg->time = (mode == MODE_PRACTICE) ? 0 : l->time;
    
    /* clear other fields */
    lg->state = GAME_NONE;
    lg->coins = 0;
    lg->timer = lg->time;
    lg->coin_rank = lg->goal_rank = lg->time_rank = 
	    lg->score_rank = lg-> times_rank = 3;
    lg->win = lg->dead = 0;
    lg->next_level = NULL;
    
    return demo_play_init(USER_REPLAY_FILE, l, lg);
}


void level_play_single(const char *filename)
/* Prepare to play a single level */
{
    struct level *l = &single_level;
    level_load(filename, l);
    level_play(l, MODE_SINGLE);
}

void level_play(const struct level *l, int m)
/* Prepare to play a level sequence from the `i'th level */
{
    struct level_game *lg = &current_level_game; 
    memset(lg, 0, sizeof(struct level_game));
    lg->mode  = m;
    lg->level = l;
    lg->balls = 3;
}

/*---------------------------------------------------------------------------*/

const struct level_game * curr_lg(void)
{
    return &current_level_game;
}

int count_extra_balls(int old_score, int coins)
{
    int modulo = old_score % 100;
    int sum    = modulo + coins;
    return sum / 100;
}

void level_stop(int state, int state_value, int clock, int coins)
/* Stop the current playing level */
{
    struct level_game * lg = &current_level_game;
    int mode = lg->mode;
    int timer = (mode == MODE_PRACTICE || mode == MODE_SINGLE) ? clock : lg->time - clock;

    lg->state = state;
    lg->coins = coins;
    lg->timer = timer;
    lg->state_value = state_value; 
   
    /* Performs challenge mode opperations */ 
    if (mode == MODE_CHALLENGE)
    {
	/* sum time */
	lg->times += timer; 
	    
	/* sum coins an earn extra balls */
	if (state == GAME_GOAL || state == GAME_SPEC || lg->level->is_bonus)
	{
	    lg->balls += count_extra_balls(lg->score, coins);
	    lg->score += coins;
	}

	/* lose ball and game */
	else /* if ((state == GAME_TIME || state == GAME_FALL) && !lg->level->is_bonus) */
	{
	    lg->balls--;
	    lg->dead = (lg->balls <= 0);
	}
    }
    
    /* Update high-scores and next level */
    set_finish_level(lg, config_simple_get_s(CONFIG_PLAYER));

    /* stop demo recording */	
    demo_play_stop(lg);
}

void level_next(void)
{
    struct level_game *lg = &current_level_game;
    lg->level = lg->next_level;
}

void level_update_player_name(void)
{
    score_change_name(&current_level_game, config_simple_get_s(CONFIG_PLAYER));
}

/*---------------------------------------------------------------------------*/

