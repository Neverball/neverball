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

int level_load(const char *filename, struct level *level)
/* Load the sol file 'filename' and fill the 'level' structure
 * return 1 on success, 0 on error
 * TODO: Currently this function does nothing since metadata are not stored
 *       in the sol file.  Therefore the sol file in not loaded */
{
    FILE *fp;

    /* Try to load the sol file */
    fp = fopen(filename, FMODE_RB);
    if (fp == NULL)
    {
	fprintf(stderr, "Error while loading level file '%s': ", filename);
	perror(NULL);
	return 0;
    }
    fclose(fp);
    
    /* Set filename */
    strcpy(level->file, filename);
    
    /* Init hs with default values */
    score_init_hs(&level->time_score, 59999, 0);
    score_init_hs(&level->goal_score, 59999, 0);
    score_init_hs(&level->coin_score, 59999, 0);
    /* Consider that true HS are set latter by the caller */

    /* Raz set info */
    level->set        = NULL;
    level->number     = 0;
    level->numbername = "0";
    level->is_locked  = 0;
    level->is_bonus   = 0;

    /* Considers that internal data are set by the caller */
			    
    return 1;
}

void level_dump_info(const struct level * level)
{
    printf("filename:        %s\n"
           "background:      %s\n"
	   "gradiant:        %s\n"
	   "screenshot:      %s\n"
	   "song:            %s\n"
	   "time limit:      %d\n"
	   "goal count:      %d\n",
	   level->file, level->back, level->grad, level->shot, level->song,
	   level->time, level->goal);
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
