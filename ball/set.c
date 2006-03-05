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
#include <assert.h>
#include <errno.h>

#include "glext.h"
#include "config.h"
#include "image.h"
#include "set.h"
#include "game.h"

/*---------------------------------------------------------------------------*/

static int count;                    /* number of sets */

static struct set set_v[MAXSET];     /* array of sets */

static struct set *current_set;      /* currently selected set */

static struct level level_v[MAXLVL]; /* levels of the current set  */

/*---------------------------------------------------------------------------*/

static void put_score(FILE *fp, const struct score *s)
{
    int j;
    for (j = 0; j < NSCORE; j++)
       fprintf(fp, "%d %d %s\n", s->timer[j], s->coins[j], s->player[j]);
}

static void set_store_hs(void)
/* Store the score of the set */
{
    const struct set *s = current_set;
    FILE *fout;
    int i;
    const struct level *l;
    int lim = s->limit;
    
    if (lim <= s->count)
	lim = s->count - 1;

    if ((fout = fopen(config_user(s->user_scores), "w")))
    {
	fprintf(fout, "%d\n", s->limit);
	
	put_score(fout, &s->time_score);
	put_score(fout, &s->coin_score);

	for (i = 0 ; i <= lim ; i++)
	{
	    l = &level_v[i];
	    put_score(fout, &l->time_score);
	    put_score(fout, &l->goal_score);
	    put_score(fout, &l->coin_score);
	}

	fclose(fout);
    }
}

static int get_score(FILE *fp, struct score *s)
{
    int j;
    int res = 1;
    for (j = 0; j < NSCORE && res; j++)
    {
       res = (fscanf(fp, "%d %d %s\n",
	       &s->timer[j], &s->coins[j], s->player[j])) == 3;
    }
    return res;
}

static void set_load_hs(void)
/* Get the score of the set */
{
    struct set *s = current_set;
    FILE *fin;
    int i;
    int res = 0;
    struct level *l;
    const char *fn = config_user(s->user_scores);
    int lim;

    if ((fin = fopen(fn, "r")))
    {
        res = (fscanf(fin, "%d\n", &lim) == 1) &&
	    s->limit == lim &&
	    get_score(fin, &s->time_score) &&
	    get_score(fin, &s->coin_score);
	
        if (lim >= s->count)
	    lim = s->count - 1;
	
	for (i = 0; i <= lim && res; i++)
	{
	    l = &level_v[i];
	    res = get_score(fin, &l->time_score) &&
	          get_score(fin, &l->goal_score) &&
	          get_score(fin, &l->coin_score);
	}

	fclose(fin);
    }
    
    if (!res && errno != ENOENT)
    {
	fprintf(stderr, _("Error while loading user high-score file '%s': "), fn);
	if (errno)
	    perror(NULL);
	else
	    fprintf(stderr, _("Incorrect format\n"));
    }
}

static const char * numbernames[] = {
	"01", "02", "03", "04", "05",
	"06", "07", "08", "09", "10",
	"11", "12", "13", "14", "15",
	"16", "17", "18", "19", "20",
	N_("B1"), N_("B2"), N_("B3"), N_("B4"), N_("B5")};


static void set_init_levels(struct set *s)
/* Count levels */
{
    FILE *fin;
    char buf[MAXSTR];

    /* Load the levels list. */
    
    s->count = 0;

    if ((fin = fopen(config_data(s->init_levels), "r")))
    {
	while (count < MAXLVL && fgets(buf, MAXSTR, fin))
	    s->count++;
	fclose(fin);
    }

    /* Load the highscore level limit */
    
    s->limit = 0;
    
    if ((fin = fopen(config_user(s->user_scores), "r")))
    {
	fscanf(fin, "%d\n", &s->limit);
	if (s->limit > s->count)
	    s->limit = 0;
	fclose(fin);
    }
    
}

static void set_init_hs(void)
/* Fill set end levels hs with initial values (score-*.txt) */
{
    struct set *s = current_set;
    char buf[MAXSTR];
    FILE *fin;
    int i = 0;
    int res = 0;
    struct level *l;
    const char *fn = config_data(s->init_scores);

    /* Set some sane values in case the scores file is missing. */
    score_init_hs(&s->time_score, 359999, 0);
    score_init_hs(&s->coin_score, 359999, 0);

    /* Load the initial high scores file. */

    if ((fin = fopen(fn, "r")))
    {
	res = fgets(buf, MAXSTR, fin) != NULL;
	
	res = res && (sscanf(buf, "%d %d %d %d %d %d",
		    &s->time_score.timer[0],
		    &s->coin_score.coins[0],
		    &s->time_score.timer[1],
		    &s->coin_score.coins[1],
		    &s->time_score.timer[2],
		    &s->coin_score.coins[2]) == 6);
	
	for (i = 0; i < s->count && res; i++)
	{
	    l = &level_v[i];
	    res = (fgets(buf, MAXSTR, fin) != NULL) &&
		sscanf(buf, "%d %d %d %d %d %d %d %d %d",
			&(l->time_score.timer[0]),
			&(l->goal_score.timer[0]),
			&(l->coin_score.coins[0]),
			&(l->time_score.timer[1]),
			&(l->goal_score.timer[1]),
			&(l->coin_score.coins[1]),
			&(l->time_score.timer[2]),
			&(l->goal_score.timer[2]),
			&(l->coin_score.coins[2])) == 9;
	}

	fclose(fin);
    }
    
    if (!res)
    {
	fprintf(stderr, _("Error while loading initial high-score file '%s': "), fn);
	if (errno)
	    perror(NULL);
	else
	    fprintf(stderr, _("Incorrect format\n"));
    }
}

/*---------------------------------------------------------------------------*/

void set_init()
{
    FILE *fin;
    struct set * set;
    int res;

    current_set = NULL;
    
    count = 0;

    if ((fin = fopen(config_data(SET_FILE), "r")))
    {
	res = 1;
	while (count < MAXSET && res)
	{
	    set = &(set_v[count]);

	    /* clean the set data */
	    memset(set, 0, sizeof(struct set));
	    
	    res = fscanf(fin, "%s %s %s %s\n",
                      set->init_levels,
                      set->init_scores,
                      set->user_scores,
                      set->shot) == 4 &&
               fgets(set->name, MAXSTR, fin) &&
               fgets(set->desc, MAXSTR, fin);
	    if (res)
	    {
                char *p = set->name + strlen(set->name) - 1;
		char *q = set->desc + strlen(set->desc) - 1;
		set->number = count;

		if (*p == '\n') *p = 0;
		if (*q == '\n') *q = 0;

		set_init_levels(set);
	   
		count++;
	    }
        }

        fclose(fin);
    }
}

/*---------------------------------------------------------------------------*/

int  set_exists(int i)
{
    return (0 <= i && i < count);
}

const struct set *get_set(int i)
{
    return set_exists(i) ? &set_v[i] : NULL;
}

/*---------------------------------------------------------------------------*/

int  set_extra_bonus_opened(const struct set *s)
/* Are extra bonus openned (ie challenge completed)? */
{
    return s->limit >= 20;
}

int  set_completed(const struct set *s)
/* Are all levels (even extra bonus) completed? */
{
    return s->limit >= s->count;
}

int  set_level_exists(const struct set *s, int i)
/* Is the level i of the set exists */
{
    return (i >= 0) && (i < s->count);
}

/*---------------------------------------------------------------------------*/

static void set_load_levels(void)
/* Load more the levels of the current set */
{
    FILE *fin;
    char buf[MAXSTR];
    char name[MAXSTR];
    struct level * l;

    int i=0, res;
    
    if ((fin = fopen(config_data(current_set->init_levels), "r")))
    {
	res = 1;
	for(i=0; i<current_set->count && res; i++)
	{
	    l = &level_v[i];
	    res = (fgets(buf, MAXSTR, fin) != NULL) &&
		(sscanf(buf, "%s %s %s %s %d %d %s",
			name,
			l->back,
			l->shot,
			l->grad,
			&l->time,
			&l->goal,
			l->song) == 7);
	    assert(res);
	    level_load(config_data(name), l);

	    /* Initialize set related info */
	    l->set        = current_set;
	    l->number     = i;
	    l->numbername = numbernames[i];
	    l->is_locked  = i > current_set->limit;
	    l->is_bonus   = i >= 20;

	    /* hs are done latter */
	}
	fclose(fin);
    }
    assert(i == current_set->count);
}

void set_goto(int i)
{
    assert(set_exists(i));
    current_set = &set_v[i];
    set_load_levels();
    set_init_hs();
    set_load_hs();
}

const struct set *curr_set(void)
{
    return current_set;
}

const struct level *get_level(int i)
{
    return (i>=0 && i<current_set->count) ? &level_v[i] : NULL;
}

/*---------------------------------------------------------------------------*/

static int score_time_comp(const struct score *S, int i, int j)
{
    if (S->timer[i] <  S->timer[j])
        return 1;

    if (S->timer[i] == S->timer[j] &&
        S->coins[i] >  S->coins[j])
        return 1;

    return 0;
}

static int score_coin_comp(const struct score *S, int i, int j)
{
    if (S->coins[i] >  S->coins[j])
        return 1;

    if (S->coins[i] == S->coins[j] &&
        S->timer[i] <  S->timer[j])
        return 1;

    return 0;
}

static void score_swap(struct score *S, int i, int j)
{
    char player[MAXNAM];
    int  tmp;

    strncpy(player,       S->player[i], MAXNAM);
    strncpy(S->player[i], S->player[j], MAXNAM);
    strncpy(S->player[j], player,       MAXNAM);

    tmp         = S->timer[i];
    S->timer[i] = S->timer[j];
    S->timer[j] = tmp;

    tmp         = S->coins[i];
    S->coins[i] = S->coins[j];
    S->coins[j] = tmp;
}

static int score_time_insert(struct score *s, const char* player, int timer, int coins)
{
    int i;
    
    strncpy(s->player[3], player, MAXNAM);
    s->timer[3] = timer;
    s->coins[3] = coins;

    for (i = 2; i >= 0 && score_time_comp(s, i + 1, i); i--)
        score_swap(s, i + 1, i);
    return i+1;
}

static int score_coin_insert(struct score *s, const char* player, int timer, int coins)
{
    int i;
    
    strncpy(s->player[3], player, MAXNAM);
    s->timer[3] = timer;
    s->coins[3] = coins;

    for (i = 2; i >= 0 && score_coin_comp(s, i + 1, i); i--)
        score_swap(s, i + 1, i);
    return i+1;
}

static int level_score_update(struct level_game *lg, const char *player)
/* Update the level score rank according to coins and timer */
{
    int timer = lg->timer;
    int coins = lg->coins;
    struct level * l = &level_v[lg->level->number];

    lg->time_rank = score_time_insert(&l->time_score, player, timer, coins);
	
    if (lg->mode == MODE_CHALLENGE || lg->mode == MODE_NORMAL)
	lg->goal_rank = score_time_insert(&l->goal_score, player, timer, coins);
    else
	lg->goal_rank = 3;

    lg->coin_rank = score_coin_insert(&l->coin_score, player, timer, coins);

    return (lg->time_rank < 3 || lg->goal_rank < 3 || lg->coin_rank < 3);
}

static int set_score_update(struct level_game *lg, const char *player)
/* Update the set score rank according to score and times */
{
    int timer = lg->times;
    int coins = lg->score;
    struct set * s = current_set;

    lg->score_rank = score_time_insert(&s->time_score, player, timer, coins);
    lg->times_rank = score_time_insert(&s->coin_score, player, timer, coins);
    return (lg->score_rank < 3 || lg->times_rank < 3);
}


void score_change_name(struct level_game *lg, const char *player)
/* Update the player name for set and level high-score */
{
#define UPDATE(i, x) (strncpy((x).player[(i)], player, MAXNAM))
    struct set * s = current_set;
    struct level *l = &level_v[lg->level->number];
    UPDATE(lg->time_rank, l->time_score);
    UPDATE(lg->goal_rank, l->goal_score);
    UPDATE(lg->coin_rank, l->coin_score);
    UPDATE(lg->score_rank, s->coin_score);
    UPDATE(lg->times_rank, s->time_score);
    set_store_hs();
}

void set_finish_level(struct level_game *lg, const char *player)
/* Inform the set that a level is finished. 
 * Update next_level and score rank fields */
{
    struct set *s = current_set;
    int level = lg->level->number;
    int dirty = 0;
    
    /* Update scores */
    dirty = level_score_update(lg, player);
    dirty = set_score_update(lg, player) || dirty;
    
    /* compute the next level */    
    if (s == NULL)
    {
        /* if no set, return */
	lg->next_level = NULL;
	return;
    }
   
    level++; /* level is the next level */
    
    /* if the next level is not oppened */
    if (s->limit < level)
        if ((lg->mode == MODE_CHALLENGE) ||
		(lg->mode == MODE_NORMAL && (level < 20 || level > 20)))
	{
	    level_v[level].is_locked = 0;
	    s->limit = level;
	    dirty = 1;
	}      
   
    /* got the next level */ 
    if (lg->mode == MODE_CHALLENGE && level >= 20)
	lg->next_level = NULL; /* End the challenge */
    else if (level < s->count && level <= s->limit)
	lg->next_level = &level_v[level];
    else
	lg->next_level = NULL;

    /* Update file */
    if (dirty)
	set_store_hs();
}

/*---------------------------------------------------------------------------*/

void level_snap(int i)
{
    char filename[MAXSTR];

    /* Convert the level name to a BMP filename. */

    memset(filename, 0, MAXSTR);
    strncpy(filename, level_v[i].file, strcspn(level_v[i].file, "."));
    strcat(filename, ".bmp");

    /* Initialize the game for a snapshot. */

    if (game_init(&level_v[i], 0, 0))
    {
        /* Render the level and grab the screen. */

        config_clear();
        game_set_fly(1.f);
        game_kill_fade();
        game_draw(1, 0);
        SDL_GL_SwapBuffers();

        image_snap(filename, config_get_d(CONFIG_WIDTH), config_get_d(CONFIG_HEIGHT));
    }
}

void set_cheat(void)
/* Open each level of the current set */
{
    current_set->limit = current_set->count;
}


/*---------------------------------------------------------------------------*/

