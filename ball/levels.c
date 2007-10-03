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

/* Currently playing level. */
static struct level_game current_level_game;

/*---------------------------------------------------------------------------*/

int level_replay(const char *filename)
{
    return demo_replay_init(filename, &current_level_game);
}

int level_play(const struct level *l, int m)
{
    struct level_game *lg = &current_level_game;

    memset(lg, 0, sizeof (struct level_game));

    lg->mode  = m;
    lg->level = l;
    lg->balls = 3;

    lg->goal = (lg->mode == MODE_PRACTICE) ? 0 : lg->level->goal;
    lg->time = (lg->mode == MODE_PRACTICE) ? 0 : lg->level->time;

    /* Clear other fields. */

    lg->status = GAME_NONE;
    lg->coins = 0;
    lg->timer = lg->time;
    lg->coin_rank = lg->goal_rank = lg->time_rank =
        lg->score_rank = lg->times_rank = 3;

    lg->win = lg->dead = lg->unlock = 0;
    lg->next_level = NULL;

    return demo_play_init(USER_REPLAY_FILE, lg->level, lg);
}

/*---------------------------------------------------------------------------*/

const struct level_game *curr_lg(void)
{
    return &current_level_game;
}

int count_extra_balls(int old_score, int coins)
{
    return ((old_score % 100) + coins) / 100;
}

void level_stat(int status, int clock, int coins)
{
    struct level_game *lg = &current_level_game;

    int mode = lg->mode;
    int timer = (mode == MODE_PRACTICE) ? clock : lg->time - clock;

    char player[MAXNAM];

    config_get_s(CONFIG_PLAYER, player, MAXNAM);

    lg->status = status;
    lg->coins = coins;
    lg->timer = timer;

    if (mode == MODE_CHALLENGE)
    {
        /* sum time */
        lg->times += timer;

        /* sum coins an earn extra balls */
        if (status == GAME_GOAL || lg->level->is_bonus)
        {
            lg->balls += count_extra_balls(lg->score, coins);
            lg->score += coins;
        }

        /* lose ball and game */
        else
        {
            lg->dead = (lg->balls <= 0);
            lg->balls--;
        }
    }

    set_finish_level(lg, player);

    demo_play_stat(lg);
}

void level_stop(void)
{
    demo_play_stop();
}

int level_next(void)
{
    struct level_game *lg = &current_level_game;

    level_stop();
    lg->level = lg->next_level;
    return level_play(lg->level, lg->mode);
}

int level_same(void)
{
    level_stop();
    return level_play(curr_lg()->level, curr_lg()->mode);
}

/*---------------------------------------------------------------------------*/

void level_update_player_name(void)
{
    char player[MAXNAM];

    config_get_s(CONFIG_PLAYER, player, MAXNAM);

    score_change_name(&current_level_game, player);
}

/*---------------------------------------------------------------------------*/
