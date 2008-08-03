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
#include "text.h"
#include "set.h"
#include "game.h"
#include "common.h"

/*---------------------------------------------------------------------------*/

struct set
{
    char file[PATHMAX];

    char *id;                  /* Internal set identifier    */
    char *name;                /* Set name                   */
    char *desc;                /* Set description            */
    char *shot;                /* Set screen-shot            */

    char user_scores[PATHMAX]; /* User high-score file       */

    struct score coin_score;   /* Challenge score            */
    struct score time_score;   /* Challenge score            */

    /* Level info */

    int   count;                /* Number of levels           */
    char *level_name_v[MAXLVL]; /* List of level file names   */
};

static int set_state = 0;

static int set;
static int count;

static struct set   set_v[MAXSET];
static struct level level_v[MAXLVL];

/*---------------------------------------------------------------------------*/

static void put_score(FILE *fp, const struct score *s)
{
    int j;

    for (j = 0; j < NSCORE; j++)
        fprintf(fp, "%d %d %s\n", s->timer[j], s->coins[j], s->player[j]);
}

void set_store_hs(void)
{
    const struct set *s = &set_v[set];
    FILE *fout;
    int i;
    const struct level *l;
    char states[MAXLVL + 1];

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
        res = fscanf(fp, "%d %d %s\n",
                     &s->timer[j],
                     &s->coins[j],
                     s->player[j]) == 3;
    }
    return res;
}

/* Get the score of the set. */
static void set_load_hs(void)
{
    struct set *s = &set_v[set];
    FILE *fin;
    int i;
    int res = 0;
    struct level *l;
    const char *fn = config_user(s->user_scores);
    char states[MAXLVL + 1];

    if ((fin = fopen(fn, "r")))
    {
        res = fscanf(fin, "%s\n", states) == 1 && strlen(states) == s->count;

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
                break;

            case 'O':
                level_v[i].is_locked = 0;
                level_v[i].is_completed = 0;
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

    if (!res && errno != ENOENT)
    {
        fprintf(stderr,
                L_("Error while loading user high-score file '%s': %s\n"),
                fn, errno ? strerror(errno) : L_("Incorrect format"));
    }
}

/*---------------------------------------------------------------------------*/

static int set_load(struct set *s, const char *filename)
{
    FILE *fin;
    char *scores, *level_name;

    fin = fopen(config_data(filename), "r");

    if (!fin)
    {
        fprintf(stderr, L_("Cannot load the set file '%s': %s\n"),
                filename, strerror(errno));
        return 0;
    }

    memset(s, 0, sizeof (struct set));

    /* Set some sane values in case the scores are missing. */

    score_init_hs(&s->time_score, 359999, 0);
    score_init_hs(&s->coin_score, 359999, 0);

    strncpy(s->file, filename, PATHMAX - 1);

    if (read_line(&s->name, fin) &&
        read_line(&s->desc, fin) &&
        read_line(&s->id,   fin) &&
        read_line(&s->shot, fin) &&
        read_line(&scores,  fin))
    {
        sscanf(scores, "%d %d %d %d %d %d",
               &s->time_score.timer[0],
               &s->time_score.timer[1],
               &s->time_score.timer[2],
               &s->coin_score.coins[0],
               &s->coin_score.coins[1],
               &s->coin_score.coins[2]);

        free(scores);

        strncpy(s->user_scores, "neverballhs-", PATHMAX - 1);
        strncat(s->user_scores, s->id, PATHMAX - 1 - strlen("neverballhs-"));

        s->count = 0;

        while (s->count < MAXLVL && read_line(&level_name, fin))
        {
            s->level_name_v[s->count] = level_name;
            s->count++;
        }

        fclose(fin);

        return 1;
    }

    free(s->name);
    free(s->desc);
    free(s->id);
    free(s->shot);

    fclose(fin);

    return 0;
}

int set_init()
{
    FILE *fin;
    char *name;

    if (set_state)
        set_free();

    set   = 0;
    count = 0;

    if ((fin = fopen(config_data(SET_FILE), "r")))
    {
        while (count < MAXSET && read_line(&name, fin))
        {
            if (set_load(&set_v[count], name))
                count++;

            free(name);
        }
        fclose(fin);

        set_state = 1;
    }

    return count;
}

void set_free(void)
{
    int i, j;

    for (i = 0; i < count; i++)
    {
        free(set_v[i].name);
        free(set_v[i].desc);
        free(set_v[i].id);
        free(set_v[i].shot);

        for (j = 0; j < set_v[i].count; j++)
            free(set_v[i].level_name_v[j]);
    }

    set_state = 0;
}

/*---------------------------------------------------------------------------*/

int set_exists(int i)
{
    return (0 <= i && i < count);
}

const char *set_id(int i)
{
    return set_exists(i) ? set_v[i].id : NULL;
}

const char *set_name(int i)
{
    return set_exists(i) ? _(set_v[i].name) : NULL;
}

const char *set_desc(int i)
{
    return set_exists(i) ? _(set_v[i].desc) : NULL;
}

const char *set_shot(int i)
{
    return set_exists(i) ? set_v[i].shot : NULL;
}

const struct score *set_time_score(int i)
{
    return set_exists(i) ? &set_v[i].time_score : NULL;
}

const struct score *set_coin_score(int i)
{
    return set_exists(i) ? &set_v[i].coin_score : NULL;
}

/*---------------------------------------------------------------------------*/

int set_level_exists(int s, int i)
{
    return (i >= 0 && i < set_v[s].count);
}

static void set_load_levels(void)
{
    struct level *l;
    int nb = 1, bnb = 1;

    int i;

    const char *roman[] = {
        "",
        "I",   "II",   "III",   "IV",   "V",
        "VI",  "VII",  "VIII",  "IX",   "X",
        "XI",  "XII",  "XIII",  "XIV",  "XV",
        "XVI", "XVII", "XVIII", "XIX",  "XX",
        "XXI", "XXII", "XXIII", "XXIV", "XXV"
    };

    for (i = 0; i < set_v[set].count; i++)
    {
        l = &level_v[i];

        level_load(set_v[set].level_name_v[i], l);

        l->set    = &set_v[set];
        l->number = i;

        if (l->is_bonus)
            sprintf(l->name, "%s",   roman[bnb++]);
        else
            sprintf(l->name, "%02d", nb++);

        l->is_locked    = 1;
        l->is_completed = 0;
    }

    /* Unlock first level. */

    level_v[0].is_locked = 0;
}

void set_goto(int i)
{
    set = i;

    set_load_levels();
    set_load_hs();
}

int curr_set(void)
{
    return set;
}

struct level *get_level(int i)
{
    return (i >= 0 && i < set_v[set].count) ? &level_v[i] : NULL;
}

/*---------------------------------------------------------------------------*/

int set_score_update(int timer, int coins, int *score_rank, int *times_rank)
{
    struct set *s = &set_v[set];
    char player[MAXSTR] = "";

    config_get_s(CONFIG_PLAYER, player, MAXSTR);

    if (score_rank)
        *score_rank = score_coin_insert(&s->coin_score, player, timer, coins);

    if (times_rank)
        *times_rank = score_time_insert(&s->time_score, player, timer, coins);

    if ((score_rank && *score_rank < 3) || (times_rank && *times_rank < 3))
        return 1;
    else
        return 0;
}

void set_rename_player(int score_rank, int times_rank, const char *player)
{
    struct set *s = &set_v[set];

    strncpy(s->coin_score.player[score_rank], player, MAXNAM);
    strncpy(s->time_score.player[times_rank], player, MAXNAM);
}

/*---------------------------------------------------------------------------*/

void level_snap(int i)
{
    char filename[MAXSTR];
    char *ext;

    /* Convert the level name to a PNG filename. */

    memset(filename, 0, MAXSTR);

    ext = strrchr(level_v[i].file, '.');
    strncpy(filename, level_v[i].file,
            ext ? ext - level_v[i].file : strlen(level_v[i].file));
    strcat(filename, ".png");

    /* Initialize the game for a snapshot. */

    if (game_init(level_v[i].file, 0, 1))
    {
        /* Render the level and grab the screen. */

        config_clear();
        game_set_fly(1.f);
        game_kill_fade();
        game_draw(1, 0);

        image_snap(filename);

        SDL_GL_SwapBuffers();
    }
}

void set_cheat(void)
{
    int i;

    for (i = 0; i < set_v[set].count; i++)
        level_v[i].is_locked = 0;
}

/*---------------------------------------------------------------------------*/
