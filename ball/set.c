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
    
    if ((fout = fopen(config_user(s->user_scores), "w")))
    {
	int code = 0;
	for (i = s->count - 1 ; i >=0 ; i--)
	{
	    code <<= 1;
	    if (!level_v[i].is_locked)
	         code |= 1;
	}
	fprintf(fout, "%d\n", code);
	
	put_score(fout, &s->time_score);
	put_score(fout, &s->coin_score);

	for (i = 0 ; i < s->count ; i++)
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

    if ((fin = fopen(fn, "r")))
    {
	int code = 0;
        res = (fscanf(fin, "%d\n", &code) == 1);
	for (i = 0 ; i < s->count ; i++)
	{
	    if (code & 1)
	        level_v[i].is_locked = 0;
	    code >>= 1;
	}
        res = res && (code != 0) &&
	    get_score(fin, &s->time_score) &&
	    get_score(fin, &s->coin_score);
	
	for (i = 0; i <= s->count && res; i++)
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

static char* chomp(char *str)
/* Remove trailing \n if any */
{
    char *p = str + strlen(str) - 1;
    if (p >= str && *p == '\n') *p = 0;
    return str;
}

static int set_load(struct set *s, const char *filename)
/* Count levels */
{
    FILE *fin;
    char buf[MAXSTR];
    int res = 0;
    
    /* Open the datafile */

    fin = fopen(filename, "r");
    if (fin == NULL)
    {
	fprintf(stderr, _("Cannot load the set file '%s':"), filename);
	perror(NULL);
	return 0;
    }

    /* Raz the set structure */

    memset(s, 0, sizeof(struct set));

    /* Set some sane values in case the scores hs is missing. */
    
    score_init_hs(&s->time_score, 359999, 0);
    score_init_hs(&s->coin_score, 359999, 0);
    
    /* Load set metadata */
    
    strcpy(s->file, filename);
    if ((res = fgets(buf, MAXSTR, fin) != NULL))
	strcpy(s->name, chomp(buf));
    if (res && (res = fgets(buf, MAXSTR, fin) != NULL))
	strcpy(s->desc, chomp(buf));
    if (res && (res = fgets(buf, MAXSTR, fin) != NULL))
	strcpy(s->setname, chomp(buf));
    if (res && (res = fgets(buf, MAXSTR, fin) != NULL))
	strcpy(s->shot, chomp(buf));
    if (res && (res = fgets(buf, MAXSTR, fin) != NULL))
	sscanf(buf, "%d %d %d %d %d %d", 
		&s->time_score.timer[0],
	       	&s->time_score.timer[1],
	       	&s->time_score.timer[2],
		&s->coin_score.coins[0],
	       	&s->coin_score.coins[1],
	       	&s->coin_score.coins[2]);
    strcpy(s->user_scores, "neverballhs-");
    strcat(s->user_scores, s->setname);

    /* Count levels levels. */
    
    s->count = 0;

    while (s->count < MAXLVL && fgets(buf, MAXSTR, fin))
	s->count++;
    
    /* Close the file, since it's no more needed */
    
    fclose(fin);

    /* Load the level limit (stored in the user highscore file) */
   /* 
    s->limit = 0;
    
    if ((fin = fopen(config_user(s->user_scores), "r")))
    {
	fscanf(fin, "%d\n", &s->limit);
	if (s->limit > s->count)
	    s->limit = 0;
	fclose(fin);
    }*/

    return 1;
}

/*---------------------------------------------------------------------------*/

void set_init()
{
    FILE *fin;
    struct set * set;
    char filename[MAXSTR];
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
	    
	    res = (fgets(filename, MAXSTR, fin) != NULL);
	    if (res)
	    {
		chomp(filename);

		res = set_load(set, config_data(filename));
		if (res)
		{
		    set->number = count;
		    count++;
		}
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

int  set_completed(const struct set *s)
/* Are all levels (even extra bonus) completed? */
{
    return 0; /*s->limit >= s->count;*/
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
    int nb=1, bnb=1;
    
    fin = fopen(current_set->file, "r");
    assert(fin != NULL);
    
    res = 1;
    /* Skip the five first lines */
    for(i=0; i<5; i++)
	fgets(buf, MAXSTR, fin);
    for(i=0; i<current_set->count && res; i++)
    {
	l = &level_v[i];
	res = (fgets(buf, MAXSTR, fin) != NULL) &&
	    (sscanf(buf, "%s", name) == 1);
	assert(res);

	level_load(config_data(name), l);

	/* Initialize set related info */
	l->set        = current_set;
	l->number     = i;
	if (l->is_bonus)
	    sprintf(l->numbername, "B%d", bnb++);
	else
	    sprintf(l->numbername, "%02d", nb++);
	l->is_locked  = 1; /*i > current_set->limit;*/
    }	
    fclose(fin);
    assert(i == current_set->count);
}

void set_goto(int i)
{
    assert(set_exists(i));
    current_set = &set_v[i];
    set_load_levels();
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
    if (level_v[level].is_locked)
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
    else if (level < s->count && !level_v[level].is_locked)
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
    int i;
    for (i=0; i < current_set->count; i++)
	level_v[i].is_locked = 0;
}


/*---------------------------------------------------------------------------*/

