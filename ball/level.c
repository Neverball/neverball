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
    for (i = 0; i < NSCORE+1; i++)
    {
	s->timer[i] = timer;
	s->coins[i] = coins;
    }
}

/*---------------------------------------------------------------------------*/

static int level_scan_metadata(struct level *l, char * av)
{
#define CASE(x) (strcmp((x), c) == 0)
    char *c    = av;
    char *stop = av + strlen(av);
    char *v, *e;
    while (c < stop)
    {
	/* look for the start of the value */
	v = strchr(c, '=');
	if (v==NULL)
	    return 0;
	*v = '\0';
	v++;
	
	/* look the end of the value */
	e = strchr(v, '\n');
	if (e==NULL)
	    return 0;
	*e = '\0';
	e ++;

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
	    l->coin_score.coins[2] = l->goal;
	}
	else if (CASE("time"))
	{
	    l->time = atoi(v);
	    l->time_score.timer[2] = l->time;
	    l->goal_score.timer[2] = l->time;
	}
	else if (CASE("time_hs"))
	    sscanf(v, "%d %d",
		    &l->time_score.timer[0],
		    &l->time_score.timer[1]);
	else if (CASE("goal_hs"))
	    sscanf(v, "%d %d",
		    &l->goal_score.timer[0],
		    &l->goal_score.timer[1]);
	else if (CASE("coin_hs"))
	    sscanf(v, "%d %d",
		    &l->coin_score.coins[0],
		    &l->coin_score.coins[1]);
	else if (CASE("levelname"))
	    strcpy(l->name, v);
	else if (CASE("version"))
	    l->version = atoi(v);
	else if (CASE("author"))
	    strcpy(l->author, v);
	else if (CASE("special"))
	    l->is_bonus = atoi(v);
	/*else
	    fprintf(stderr, "File %s, ignore %s metadata.\n", l->file, c);*/

	c = e;
    }
    return 1;
}

int level_load(const char *filename, struct level *level)
/* Load the sol file 'filename' and fill the 'level' structure
 * return 1 on success, 0 on error */
{
    struct s_file sol; /* The solid file data */
    int i;
    int money; /* sum of coin value */
   
    /* raz level */
    memset(level, 0, sizeof(struct level));
    
    memset(&sol, 0, sizeof(sol));

    /* Try to load the sol file */
    if (!sol_load_only_file(&sol, filename))
    {
	fprintf(stderr, "Error while loading level file '%s': ", filename);
        if (errno)
	   perror(NULL);
	else
	   fprintf(stderr, _("Not a valid level file\n"));
	return 0;
    }

    /* Set filename */
    strcpy(level->file, filename);
    
    /* Init hs with default values */
    score_init_hs(&level->time_score, 59999, 0);
    score_init_hs(&level->goal_score, 59999, 0);
    score_init_hs(&level->coin_score, 59999, 0);

    /* Compute money and default max money */
    money = 0;
    for (i = 0; i < sol.cc; i++)
	money += sol.cv[i].n;
    level->coin_score.coins[0] = money;
    
    /* Scan sol metadata */
    if (sol.ac > 0)
        level_scan_metadata(level, sol.av);

    /* Compute initial hs default values */
#define HOP(t, c) if (t[2] c t[0]) t[0] = t[1] = t[2]; else if (t[2] c t[1]) t[1] = (t[0] + t[2]) / 2
    HOP(level->time_score.timer, <=);
    HOP(level->goal_score.timer, <=);
    HOP(level->coin_score.coins, >=);

    /* Free the sol structure, no more needed */    
    sol_free(&sol);

    return 1;
}

/*---------------------------------------------------------------------------*/

void level_dump_info(const struct level *l)
/* This function dump the info of a demo structure
 * It's only a function for debugging, no need of I18N */
{
    printf("filename:        %s\n"
	   "name:            %s\n"
	   "version:         %d\n"
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
	   l->file, l->name, l->version, l->author,
	   l->time, l->goal,
	   l->time_score.timer[0],
	   l->time_score.timer[1],
	   l->time_score.timer[2],
	   l->goal_score.timer[0],
	   l->goal_score.timer[1],
	   l->goal_score.timer[2],
	   l->coin_score.coins[0],
	   l->coin_score.coins[1],
	   l->coin_score.coins[2],
	   l->message, l->back, l->grad, l->shot, l->song);
}

/*---------------------------------------------------------------------------*/

const char * mode_to_str(int m)
{
    switch (m)
    {
    case MODE_CHALLENGE: return _("Challenge");
    case MODE_NORMAL:    return _("Normal");
    case MODE_PRACTICE:  return _("Practice");
    case MODE_SINGLE:    return _("Single");
    default:             return "???";
    }
}

/*---------------------------------------------------------------------------*/

const char * state_to_str(int m)
{
    switch (m)
    {
    case GAME_NONE:    return _("Aborted");
    case GAME_TIME:    return _("Time-out");
    case GAME_GOAL:    return _("Success");
    case GAME_FALL:    return _("Fall-out");
    default:           return "???";
    }
}

/*---------------------------------------------------------------------------*/
