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

/*---------------------------------------------------------------------------*/

static void put_score(FILE *fp, const struct score *s)
{
    int j;
    for (j = 0; j < NSCORE; j++)
       fprintf(fp, "%d %d %s\n", s->timer[j], s->coins[j], s->player[j]);
}

static void set_store_hs(const struct set *s)
/* Store the score of the set */
{
    FILE *fout;
    int i;
    const struct level *l;
    char states[MAXLVL + 1];
    struct level *level_v = s->level_v;

    if ((fout = fopen(config_user(s->user_scores), "w")))
    {
        for (i = 0; i < s->count; i++)
        {
            if (level_v[i].is_locked)
                states[i] = 'L';
            else if (level_v[i].is_completed)
                states[i] = 'C';
            else
                states[i] = 'O';
        }
        states[s->count] = '\0';
        fprintf(fout, "%s\n",states);

        put_score(fout, &s->time_score);
        put_score(fout, &s->coin_score);

        for (i = 0; i < s->count; i++)
        {
            l = &level_v[i];
            put_score(fout, &l->score.best_times);
            put_score(fout, &l->score.unlock_goal);
            put_score(fout, &l->score.most_coins);
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

static void set_load_hs(struct set *s)
/* Get the score of the set */
{
    FILE *fin;
    int i;
    int res = 0;
    struct level *l;
    const char *fn = config_user(s->user_scores);
    char states[MAXLVL + 1];
    struct level *level_v = s->level_v;
    
    /* Load the levels states (stored in the user highscore file) */

    s->locked = s->count;
    s->completed = 0;

    if ((fin = fopen(fn, "r")))
    {
        res = ((fscanf(fin, "%s\n", states) == 1) &&
               (strlen(states) == s->count));
        for (i = 0; i < s->count && res; i++)
        {
            switch (states[i])
            {
            case 'L':
                level_v[i].is_locked = 1;
                level_v[i].is_completed = 0;
                break;

            case 'C':
                level_v[i].is_locked = 0;
                level_v[i].is_completed = 1;
                s->completed += 1;
                s->locked -= 1;
                break;

            case 'O':
                level_v[i].is_locked = 0;
                level_v[i].is_completed = 0;
                s->locked -= 1;
                break;

            default:
                res = 0;
            }
        }

        res = res &&
            get_score(fin, &s->time_score) &&
            get_score(fin, &s->coin_score);

        for (i = 0; i < s->count && res; i++)
        {
            l = &level_v[i];
            res = get_score(fin, &l->score.best_times) &&
                  get_score(fin, &l->score.unlock_goal) &&
                  get_score(fin, &l->score.most_coins);
        }

        fclose(fin);
    }
    
    s->level_v[0].is_locked = 0; /* unlock the first level */
    if (s->locked == s->count)
        s->locked = s->count-1;

    if (!res && errno != ENOENT)
    {
        fprintf(stderr,
                _("Error while loading user high-score file '%s': %s\n"),
                fn, errno ? strerror(errno) : _("Incorrect format"));
    }
}

/* Remove trailing \n if any */

static char *chomp(char *str)
{
    char *p = str + strlen(str) - 1;
    if (p >= str && *p == '\n')
        *p = 0;
    return str;
}

static int set_load(struct set *s, const char *filename)
{
    FILE *fin;
    char buf[MAXSTR];
    int res;
    struct level *l;
    char name[MAXSTR];
    int i = 0;
    int nb = 1, bnb = 1;

    fin = fopen(config_data(filename), "r");

    if (!fin)
    {
        fprintf(stderr, _("Cannot load the set file '%s': %s\n"),
                filename, strerror(errno));
        return 0;
    }

    memset(s, 0, sizeof (struct set));

    /* Set some sane values in case the scores hs is missing. */

    score_init_hs(&s->time_score, 359999, 0);
    score_init_hs(&s->coin_score, 359999, 0);

    /* Load set metadata. */

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

    /* Load levels. */

    for (i=0 ; i < MAXLVL && (res = (fscanf(fin, "%s", name) == 1)) ; i++)
    {
        l = &s->level_v[i];

        level_load(config_data(name), l);

        /* Initialize set related info */
        l->set        = s;
        l->number     = i;
        if (l->is_bonus)
            sprintf(l->repr, _("B%d"), bnb++);
        else
            sprintf(l->repr, "%02d", nb++);
        l->is_locked    = 1;
        l->is_completed = 0;
    }

    s->count = i;

    fclose(fin);
   
    /* Load scores and user level state */
    
    set_load_hs(s);

    return 1;
}

/*---------------------------------------------------------------------------*/

void set_init()
{
    FILE *fin;
    struct set *set;
    char filename[MAXSTR];

    current_set = NULL;
    count = 0;

    if ((fin = fopen(config_data(SET_FILE), "r")))
    {
        while (count < MAXSET && fgets(filename, MAXSTR, fin))
        {
            chomp(filename);
            set = &(set_v[count]);

            if (set_load(set, filename))
            {
                set->number = count;
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

int  set_unlocked(const struct set *s)
/* Are all levels (even extra bonus) unlocked? */
{
    return s->locked == 0;
}

int  set_completed(const struct set *s)
/* Are all levels (even extra bonus) completed? */
{
    return s->completed == s->count;
}

int  set_level_exists(const struct set *s, int i)
/* Does the level i of the set exist? */
{
    return (i >= 0) && (i < s->count);
}

/*---------------------------------------------------------------------------*/

void set_goto(int i)
{
    current_set = &set_v[i];
}

const struct set *curr_set(void)
{
    return current_set;
}

const struct level *get_level(int i)
{
    return (i >= 0 && i < current_set->count) ? &current_set->level_v[i] : NULL;
}

const struct level *search_level(const char *levelname)
{
    int s, l;
    for (s=0 ; s < count ; s++)
        for (l=0 ; l < set_v[s].count ; l++)
            if (strcmp(set_v[s].level_v[l].levelname, levelname) == 0)
                return &set_v[s].level_v[l];
    return NULL;
}

/*---------------------------------------------------------------------------*/

static int score_time_comp(const struct score *S, int i, int j)
{
    if (S->timer[i] < S->timer[j])
        return 1;

    if (S->timer[i] == S->timer[j] && S->coins[i] > S->coins[j])
        return 1;

    return 0;
}

static int score_coin_comp(const struct score *S, int i, int j)
{
    if (S->coins[i] > S->coins[j])
        return 1;

    if (S->coins[i] == S->coins[j] && S->timer[i] < S->timer[j])
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

static int score_time_insert(struct score *s, const char *player, int timer,
                             int coins)
{
    int i;

    strncpy(s->player[3], player, MAXNAM);
    s->timer[3] = timer;
    s->coins[3] = coins;

    for (i = 2; i >= 0 && score_time_comp(s, i + 1, i); i--)
        score_swap(s, i + 1, i);

    return i + 1;
}

static int score_coin_insert(struct score *s, const char *player, int timer,
                             int coins)
{
    int i;

    strncpy(s->player[3], player, MAXNAM);
    s->timer[3] = timer;
    s->coins[3] = coins;

    for (i = 2; i >= 0 && score_coin_comp(s, i + 1, i); i--)
        score_swap(s, i + 1, i);

    return i + 1;
}

static int level_score_update(struct level_game *lg, const char *player)
/* Update the level score rank according to coins and timer */
{
    int timer = lg->timer;
    int coins = lg->coins;
    struct level *l = &current_set->level_v[lg->level->number];

    lg->time_rank = score_time_insert(&l->score.best_times,
                                      player, timer, coins);

    if (lg->mode == MODE_CHALLENGE || lg->mode == MODE_NORMAL)
        lg->goal_rank = score_time_insert(&l->score.unlock_goal,
                                          player, timer, coins);
    else
        lg->goal_rank = 3;

    lg->coin_rank = score_coin_insert(&l->score.most_coins,
                                      player, timer, coins);

    return (lg->time_rank < 3 || lg->goal_rank < 3 || lg->coin_rank < 3);
}

static int set_score_update(struct level_game *lg, const char *player)
/* Update the set score rank according to score and times */
{
    int timer = lg->times;
    int coins = lg->score;
    struct set *s = current_set;

    lg->score_rank = score_time_insert(&s->time_score, player, timer, coins);
    lg->times_rank = score_time_insert(&s->coin_score, player, timer, coins);
    return (lg->score_rank < 3 || lg->times_rank < 3);
}


void score_change_name(struct level_game *lg, const char *player)
/* Update the player name for set and level high-score */
{
#define UPDATE(i, x) (strncpy((x).player[(i)], player, MAXNAM))
    struct set *s = current_set;
    struct level *l = &s->level_v[lg->level->number];
    UPDATE(lg->time_rank, l->score.best_times);
    UPDATE(lg->goal_rank, l->score.unlock_goal);
    UPDATE(lg->coin_rank, l->score.most_coins);
    UPDATE(lg->score_rank, s->coin_score);
    UPDATE(lg->times_rank, s->time_score);
    set_store_hs(s);
}

static struct level *next_level(int i)
{
/* Return the ith level, or NULL */
    return set_level_exists(current_set, i + 1) ? &current_set->level_v[i + 1] : NULL;
}

static struct level *next_normal_level(int i)
/* Return the next normal level (starting for i)
 * Return NULL if there is not a such level */
{
    for (i++; i < current_set->count; i++)
        if (!current_set->level_v[i].is_bonus)
            return &current_set->level_v[i];
    return NULL;
}

void set_finish_level(struct level_game *lg, const char *player)
/* Inform the set that a level is finished.
 * Update next_level and score rank fields */
{
    struct set *s = current_set;
    int ln = lg->level->number; /* curent level number */
    struct level *cl = &s->level_v[ln];    /* current level */
    struct level *nl = NULL;    /* next level */
    int dirty = 0;              /* HS should be saved? */

    assert(s == cl->set);

    /* if no set, no next level */
    if (s == NULL)
    {
        /* if no set, return */
        lg->next_level = NULL;
        return;
    }

    /* On level completed */
    if (lg->state == GAME_GOAL)
    {
        /* Update level scores */
        dirty = level_score_update(lg, player);

        /* Complete the level */
        if (lg->mode == MODE_CHALLENGE || lg->mode == MODE_NORMAL)
        {
            /* Complete the level */
            if (!cl->is_completed)
            {
                cl->is_completed = 1;
                s->completed += 1;
                dirty = 1;
            }
        }
    }

    /* On goal reached */
    if (lg->state == GAME_GOAL || lg->state == GAME_SPEC)
    {
        /* Identify the following level */
        nl = next_level(ln + lg->state_value);
        if (nl != NULL)
        {
            /* skip bonuses if unlocked in non challenge mode */
            if (nl->is_bonus && nl->is_locked && lg->mode != MODE_CHALLENGE)
                nl = next_normal_level(nl->number);
        }
        else if (lg->mode == MODE_CHALLENGE)
            lg->win = 1;
    }
    else if (cl->is_bonus || lg->mode != MODE_CHALLENGE)
    {
        /* On fail, identify the next level (only in bonus for challenge) */
        nl = next_normal_level(ln);
        /* Next level may be unavailable */
        if (!cl->is_bonus && nl != NULL && nl->is_locked)
            nl = NULL;
        /* Fail a bonus level but win the set! */
        else if (nl == NULL && lg->mode == MODE_CHALLENGE)
            lg->win = 1;
    }

    /* Win ! */
    if (lg->win)
    {
        /* update set score */
        set_score_update(lg, player);
        /* unlock all levels */
        set_cheat();
        dirty = 1;
    }

    /* unlock the next level if needed */
    if (nl != NULL && nl->is_locked)
    {
        if (lg->mode == MODE_CHALLENGE || lg->mode == MODE_NORMAL)
        {
            lg->unlock = 1;
            nl->is_locked = 0;
            s->locked -= 1;
            dirty = 1;
        }
        else
            nl = NULL;
    }

    /* got the next level */
    lg->next_level = nl;

    /* Update file */
    if (dirty)
        set_store_hs(s);
}

/*---------------------------------------------------------------------------*/

void level_snap(int i)
{
    char filename[MAXSTR];
    char *ext;
    struct level *level_v = current_set->level_v;

    /* Convert the level name to a PNG filename. */

    memset(filename, 0, MAXSTR);

    ext = strrchr(level_v[i].file, '.');
    strncpy(filename, level_v[i].file,
            ext ? ext - level_v[i].file : strlen(level_v[i].file));
    strcat(filename, ".png");

    /* Initialize the game for a snapshot. */

    if (game_init(&level_v[i], 0, 0))
    {
        int shadow;

        if ((shadow = config_get_d(CONFIG_SHADOW)))
            config_set_d(CONFIG_SHADOW, 0);

        /* Render the level and grab the screen. */

        config_clear();
        game_set_fly(1.f);
        game_kill_fade();
        game_draw(1, 0);
        SDL_GL_SwapBuffers();

        image_snap(filename);

        if (shadow)
            config_set_d(CONFIG_SHADOW, 1);
    }
}

void set_cheat(void)
/* Open each level of the current set */
{
    int i;
    current_set->locked = 0;
    for (i = 0; i < current_set->count; i++)
        current_set->level_v[i].is_locked = 0;
}


/*---------------------------------------------------------------------------*/
