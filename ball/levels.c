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

#include "level.h"
#include "levels.h"
#include "image.h"
#include "game.h"
#include "geom.h"
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
    int mode  = lg->mode;
    const struct level *l;

    if (curr_set())
	l = get_level(lg->level);
    else
        l = &single_level;	    
    
    lg->goal = (mode == MODE_PRACTICE) ? 0 : l->goal;
    lg->time = (mode == MODE_PRACTICE) ? 0 : l->time;
    
    /* clear other fields */
    lg->state = GAME_NONE;
    lg->coins = 0;
    lg->timer = lg->time;
    lg->coin_rank = lg->goal_rank = lg->time_rank = 
	    lg->score_rank = lg-> times_rank = 3;
    lg->next_level = 0;
    
    return demo_play_init(USER_REPLAY_FILE, l, lg);
}


void level_play_single(const char *filename)
/* Prepare to play a single level */
{
    struct level *l = &single_level;

    current_level_game.mode  = MODE_SINGLE;
    current_level_game.level = 0;
    
    strncpy(l->file, filename, MAXSTR);
    l->back[0] = '\0';
    l->grad[0] = '\0';
    l->song[0] = '\0';
    l->shot[0] = '\0';
    l->goal    = 0;
    l->time    = 0;
}

void level_play(int i, int m)
/* Prepare to play a level sequence from the `i'th level */
{
    current_level_game.mode = m;
    current_level_game.level = i;

    current_level_game.score = 0;
    current_level_game.balls = 3;
    current_level_game.times = 0;
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

void level_stop(int state, int clock, int coins)
/* Stop the current playing level */
{
    struct level_game * lg = &current_level_game;
    int mode = lg->mode;
    int timer = (mode == MODE_PRACTICE || mode == MODE_SINGLE) ? clock : lg->time - clock;

    lg->state = state;
    lg->coins = coins;
    lg->timer = timer;
   
    /* Performs challenge mode opperations */ 
    if (mode == MODE_CHALLENGE)
    {
	/* sum time */
	lg->times += timer; 
	    
	/* sum coins an earn extra balls */
	if (state == GAME_GOAL)
	{
	    lg->balls += count_extra_balls(lg->score, coins);
	    lg->score += coins;
	}

	/* lose ball */
        if (state == GAME_TIME || state == GAME_FALL)
	    lg->balls--;
    }
    
    /* Update high-scores and next level */
    if (state == GAME_GOAL && curr_set())
	set_finish_level(lg, config_simple_get_s(CONFIG_PLAYER));
    else
	lg->next_level = -1;

    /* stop demo recording */	
    demo_play_stop(lg);
}

int level_dead(void)
{
    int mode = current_level_game.mode;
    int balls = current_level_game.balls;
    return (mode == MODE_CHALLENGE) && (balls <= 0);
}

void level_next(void)
{
    current_level_game.level = current_level_game.next_level;
}

void level_update_player_name(void)
{
    score_change_name(&current_level_game, config_simple_get_s(CONFIG_PLAYER));
}

/*---------------------------------------------------------------------------*/

