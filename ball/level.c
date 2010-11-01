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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "common.h"
#include "solid.h"
#include "config.h"
#include "level.h"
#include "set.h"

/*---------------------------------------------------------------------------*/

static void scan_level_attribs(struct level *l, const struct s_file *fp)
{
    int i;

    int have_goal = 0;
    int have_time = 0;

    int need_time_easy = 0;
    int need_goal_easy = 0;
    int need_coin_easy = 0;

    for (i = 0; i < fp->dc; i++)
    {
        char *k = fp->av + fp->dv[i].ai;
        char *v = fp->av + fp->dv[i].aj;

        if (strcmp(k, "message") == 0)
            SAFECPY(l->message, v);
        else if (strcmp(k, "song") == 0)
            SAFECPY(l->song, v);
        else if (strcmp(k, "shot") == 0)
            SAFECPY(l->shot, v);
        else if (strcmp(k, "goal") == 0)
        {
            l->goal = atoi(v);
            have_goal = 1;
        }
        else if (strcmp(k, "time") == 0)
        {
            l->time = atoi(v);
            have_time = 1;
        }
        else if (strcmp(k, "time_hs") == 0)
        {
            switch (sscanf(v, "%d %d %d",
                           &l->scores[SCORE_TIME].timer[RANK_HARD],
                           &l->scores[SCORE_TIME].timer[RANK_MEDM],
                           &l->scores[SCORE_TIME].timer[RANK_EASY]))
            {
            case 2: need_time_easy = 1; break;
            case 3: break;
            }
        }
        else if (strcmp(k, "goal_hs") == 0)
        {
            switch (sscanf(v, "%d %d %d",
                           &l->scores[SCORE_GOAL].timer[RANK_HARD],
                           &l->scores[SCORE_GOAL].timer[RANK_MEDM],
                           &l->scores[SCORE_GOAL].timer[RANK_EASY]))
            {
            case 2: need_goal_easy = 1; break;
            case 3: break;
            }
        }
        else if (strcmp(k, "coin_hs") == 0)
        {
            switch (sscanf(v, "%d %d %d",
                           &l->scores[SCORE_COIN].coins[RANK_HARD],
                           &l->scores[SCORE_COIN].coins[RANK_MEDM],
                           &l->scores[SCORE_COIN].coins[RANK_EASY]))
            {
            case 2: need_coin_easy = 1; break;
            case 3: break;
            }
        }
        else if (strcmp(k, "version") == 0)
            SAFECPY(l->version, v);
        else if (strcmp(k, "author") == 0)
            SAFECPY(l->author, v);
        else if (strcmp(k, "bonus") == 0)
            l->is_bonus = atoi(v) ? 1 : 0;
    }

    if (have_goal)
    {
        if (need_coin_easy)
            l->scores[SCORE_COIN].coins[RANK_EASY] = l->goal;

        l->scores[SCORE_GOAL].coins[RANK_HARD] = l->goal;
        l->scores[SCORE_GOAL].coins[RANK_MEDM] = l->goal;
        l->scores[SCORE_GOAL].coins[RANK_EASY] = l->goal;
    }

    if (have_time)
    {
        if (need_time_easy)
            l->scores[SCORE_TIME].timer[RANK_EASY] = l->time;
        if (need_goal_easy)
            l->scores[SCORE_GOAL].timer[RANK_EASY] = l->time;

        l->scores[SCORE_COIN].timer[RANK_HARD] = l->time;
        l->scores[SCORE_COIN].timer[RANK_MEDM] = l->time;
        l->scores[SCORE_COIN].timer[RANK_EASY] = l->time;
    }
}

int level_load(const char *filename, struct level *level)
{
    struct s_file sol;

    memset(level, 0, sizeof (struct level));
    memset(&sol,  0, sizeof (sol));

    if (!sol_load_only_head(&sol, filename))
    {
        fprintf(stderr, L_("Failure to load level file '%s'\n"), filename);
        return 0;
    }

    SAFECPY(level->file, filename);

    score_init_hs(&level->scores[SCORE_TIME], 59999, 0);
    score_init_hs(&level->scores[SCORE_GOAL], 59999, 0);
    score_init_hs(&level->scores[SCORE_COIN], 59999, 0);

    scan_level_attribs(level, &sol);

    sol_free(&sol);

    return 1;
}

/*---------------------------------------------------------------------------*/

int level_exists(int i)
{
    return !!get_level(i);
}

void level_open(struct level *level)
{
    level->is_locked = 0;
}

int level_opened(const struct level *level)
{
    return !level->is_locked;
}

void level_complete(struct level *level)
{
    level->is_completed = 1;
}

int level_completed(const struct level *level)
{
    return level->is_completed;
}

int level_time(const struct level *level)
{
    return level->time;
}

int level_goal(const struct level *level)
{
    return level->goal;
}

int  level_bonus(const struct level *level)
{
    return level->is_bonus;
}

const char *level_shot(const struct level *level)
{
    return level->shot;
}

const char *level_file(const struct level *level)
{
    return level->file;
}

const char *level_song(const struct level *level)
{
    return level->song;
}

const char *level_name(const struct level *level)
{
    return level->name;
}

const char *level_msg(const struct level *level)
{
    if (strlen(level->message) > 0)
        return _(level->message);
    return "";
}

const struct score *level_score(struct level *level, int s)
{
    return &level->scores[s];
}

/*---------------------------------------------------------------------------*/

int level_score_update(struct level *l,
                       int timer,
                       int coins,
                       int *time_rank,
                       int *goal_rank,
                       int *coin_rank)
{
    const char *player =  config_get_s(CONFIG_PLAYER);

    score_time_insert(&l->scores[SCORE_TIME], time_rank, player, timer, coins);
    score_time_insert(&l->scores[SCORE_GOAL], goal_rank, player, timer, coins);
    score_coin_insert(&l->scores[SCORE_COIN], coin_rank, player, timer, coins);

    if ((time_rank && *time_rank < 3) ||
        (goal_rank && *goal_rank < 3) ||
        (coin_rank && *coin_rank < 3))
        return 1;
    else
        return 0;
}

void level_rename_player(struct level *l,
                         int time_rank,
                         int goal_rank,
                         int coin_rank,
                         const char *player)
{
    SAFECPY(l->scores[SCORE_TIME].player[time_rank], player);
    SAFECPY(l->scores[SCORE_GOAL].player[goal_rank], player);
    SAFECPY(l->scores[SCORE_COIN].player[coin_rank], player);
}

/*---------------------------------------------------------------------------*/

