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
#include <errno.h>

#include "config.h"
#include "demo.h"
#include "level.h"
#include "mode.h"
#include "set.h"
#include "solid.h"

/*---------------------------------------------------------------------------*/

static void scan_dict(struct level *l, const struct s_file *fp)
{
    int i;

    for (i = 0; i < fp->dc; i++)
    {
        char *k = fp->av + fp->dv[i].ai;
        char *v = fp->av + fp->dv[i].aj;

        if (strcmp(k, "message") == 0)
            strncpy(l->message, v, MAXSTR);
        else if (strcmp(k, "back") == 0)
            strncpy(l->back, v, PATHMAX);
        else if (strcmp(k, "song") == 0)
            strncpy(l->song, v, PATHMAX);
        else if (strcmp(k, "grad") == 0)
            strncpy(l->grad, v, PATHMAX);
        else if (strcmp(k, "shot") == 0)
            strncpy(l->shot, v, PATHMAX);
        else if (strcmp(k, "goal") == 0)
        {
            l->goal = atoi(v);
            l->score.most_coins.coins[2] = l->goal;
        }
        else if (strcmp(k, "time") == 0)
        {
            l->time = atoi(v);
            l->score.best_times.timer[2] = l->time;
            l->score.unlock_goal.timer[2] = l->time;
        }
        else if (strcmp(k, "time_hs") == 0)
            sscanf(v, "%d %d",
                   &l->score.best_times.timer[0],
                   &l->score.best_times.timer[1]);
        else if (strcmp(k, "goal_hs") == 0)
            sscanf(v, "%d %d",
                   &l->score.unlock_goal.timer[0],
                   &l->score.unlock_goal.timer[1]);
        else if (strcmp(k, "coin_hs") == 0)
            sscanf(v, "%d %d",
                   &l->score.most_coins.coins[0],
                   &l->score.most_coins.coins[1]);
        else if (strcmp(k, "version") == 0)
            strncpy(l->version, v, MAXSTR);
        else if (strcmp(k, "author") == 0)
            strncpy(l->author, v, MAXSTR);
        else if (strcmp(k, "bonus") == 0)
            l->is_bonus = atoi(v) ? 1 : 0;
    }
}

int level_load(const char *filename, struct level *level)
{
    struct s_file sol;

    int money;
    int i;

    memset(level, 0, sizeof (struct level));
    memset(&sol,  0, sizeof (sol));

    if (!sol_load_only_head(&sol, config_data(filename)))
    {
        fprintf(stderr,
                _("Error while loading level file '%s': %s\n"), filename,
                errno ? strerror(errno) : _("Not a valid level file"));
        return 0;
    }

    strcpy(level->file, filename);

    /* Init hs with default values */
    score_init_hs(&level->score.best_times, 59999, 0);
    score_init_hs(&level->score.unlock_goal, 59999, 0);
    score_init_hs(&level->score.most_coins, 59999, 0);

    /* Compute money and default max money */
    money = 0;
    for (i = 0; i < sol.hc; i++)
        if (sol.hv[i].t == ITEM_COIN)
            money += sol.hv[i].n;
    level->score.most_coins.coins[0] = money;

    if (sol.dc > 0)
        scan_dict(level, &sol);

    /* Compute initial hs default values */

#define HOP(t, c) \
    if (t[2] c t[0]) \
        t[0] = t[1] = t[2]; \
    else if (t[2] c t[1]) \
        t[1] = (t[0] + t[2]) / 2

    HOP(level->score.best_times.timer, <=);
    HOP(level->score.unlock_goal.timer, <=);
    HOP(level->score.most_coins.coins, >=);

    sol_free(&sol);

    return 1;
}

void level_dump(const struct level *l)
{
    printf("filename:        %s\n"
           "version:         %s\n"
           "author:          %s\n"
           "time limit:      %d\n"
           "goal count:      %d\n"
           "time hs:         %d %d %d\n"
           "goal hs:         %d %d %d\n"
           "coin hs:         %d %d %d\n"
           "message:         %s\n"
           "background:      %s\n"
           "gradient:        %s\n"
           "screenshot:      %s\n"
           "song:            %s\n",
           l->file,
           l->version,
           l->author,
           l->time,
           l->goal,
           l->score.best_times.timer[0],
           l->score.best_times.timer[1],
           l->score.best_times.timer[2],
           l->score.unlock_goal.timer[0],
           l->score.unlock_goal.timer[1],
           l->score.unlock_goal.timer[2],
           l->score.most_coins.coins[0],
           l->score.most_coins.coins[1],
           l->score.most_coins.coins[2],
           l->message,
           l->back,
           l->grad,
           l->shot,
           l->song);
}

/*---------------------------------------------------------------------------*/

int level_replay(const char *filename)
{
    return demo_replay_init(filename, curr_lg());
}

int level_play(const struct level *l, int m)
{
    struct level_game *lg = curr_lg();

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

void level_stat(int status, int clock, int coins)
{
    struct level_game *lg = curr_lg();

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
    struct level_game *lg = curr_lg();

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

int count_extra_balls(int old_score, int coins)
{
    return ((old_score % 100) + coins) / 100;
}

void level_update_player_name(void)
{
    char player[MAXNAM];

    config_get_s(CONFIG_PLAYER, player, MAXNAM);

    score_change_name(curr_lg(), player);
}

/*---------------------------------------------------------------------------*/

const char *status_to_str(int m)
{
    switch (m)
    {
    case GAME_NONE:    return _("Aborted");
    case GAME_TIME:    return _("Time-out");
    case GAME_GOAL:    return _("Success");
    case GAME_FALL:    return _("Fall-out");
    default:           return _("Unknown");
    }
}

/*---------------------------------------------------------------------------*/
