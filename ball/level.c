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

#include "level.h"
#include "solid.h"

/*---------------------------------------------------------------------------*/

static void level_scan_metadata(struct level *l, const struct s_file *fp)
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
            l->is_bonus = atoi(v);
    }
}

/* Load the sol file 'filename' and fill the 'level' structure.  Return 1 on
 * success, 0 on error. */

int level_load(const char *filename, struct level *level)
{
    struct s_file sol;

    int money;
    int i;

    memset(level, 0, sizeof (struct level));
    memset(&sol,  0, sizeof (sol));

    /* Try to load the sol file */
    if (!sol_load_only_file(&sol, config_data(filename)))
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

    /* Scan sol metadata */
    if (sol.dc > 0)
        level_scan_metadata(level, &sol);

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

/*---------------------------------------------------------------------------*/

void level_dump_info(const struct level *l)
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

const char *state_to_str(int m)
{
    switch (m)
    {
    case GAME_NONE:    return _("Aborted");
    case GAME_TIME:    return _("Time-out");
    case GAME_SPEC:
    case GAME_GOAL:    return _("Success");
    case GAME_FALL:    return _("Fall-out");
    default:           return _("Unknown");
    }
}

/*---------------------------------------------------------------------------*/
