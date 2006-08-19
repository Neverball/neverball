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

void score_init_hs(struct score *s, int timer, int coins)
{
    int i;

    strcpy(s->player[0], "Hard");
    strcpy(s->player[1], "Medium");
    strcpy(s->player[2], "Easy");
    strcpy(s->player[3], "");

    for (i = 0; i < NSCORE + 1; i++)
    {
        s->timer[i] = timer;
        s->coins[i] = coins;
    }
}

/*---------------------------------------------------------------------------*/

static int level_scan_metadata(struct level *l, char *av)
{
#define CASE(x) (strcmp((x), c) == 0)
    char *c    = av;
    char *stop = av + strlen(av);
    char *v, *e;

    while (c < stop)
    {
        /* look for the start of the value */
        v = strchr(c, '=');
        if (v == NULL)
            return 0;
        *v = '\0';
        v++;

        /* look the end of the value */
        e = strchr(v, '\n');
        if (e == NULL)
            return 0;
        *e = '\0';
        e++;

        /* test metadata */
        if (CASE("message"))
            strcpy(l->message, v);
        else if (CASE("back"))
            strcpy(l->back, v);
        else if (CASE("song"))
            strcpy(l->song, v);
        else if (CASE("grad"))
            strcpy(l->grad, v);
        else if (CASE("shot"))
            strcpy(l->shot, v);
        else if (CASE("goal"))
        {
            l->goal = atoi(v);
            l->score.most_coins.coins[2] = l->goal;
        }
        else if (CASE("time"))
        {
            l->time = atoi(v);
            l->score.best_times.timer[2] = l->time;
            l->score.unlock_goal.timer[2] = l->time;
        }
        else if (CASE("time_hs"))
            sscanf(v, "%d %d",
                   &l->score.best_times.timer[0],
                   &l->score.best_times.timer[1]);
        else if (CASE("goal_hs"))
            sscanf(v, "%d %d",
                   &l->score.unlock_goal.timer[0],
                   &l->score.unlock_goal.timer[1]);
        else if (CASE("coin_hs"))
            sscanf(v, "%d %d",
                   &l->score.most_coins.coins[0],
                   &l->score.most_coins.coins[1]);
        else if (CASE("version"))
            strcpy(l->version, v);
        else if (CASE("author"))
            strcpy(l->author, v);
        else if (CASE("special"))
            l->is_bonus = atoi(v);

        c = e;
    }
    return 1;
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

    /* Scan sol metadata */
    if (sol.ac > 0)
        level_scan_metadata(level, sol.av);

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
           "gradiant:        %s\n"
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

const char *mode_to_str(int m)
{
    switch (m)
    {
    case MODE_CHALLENGE: return _("Challenge");
    case MODE_NORMAL:    return _("Normal");
    case MODE_PRACTICE:  return _("Practice");
    case MODE_SINGLE:    return _("Single");
    default:             return _("Unknown");
    }
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
