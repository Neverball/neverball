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

#include "glext.h"
#include "config.h"
#include "video.h"
#include "image.h"
#include "set.h"
#include "common.h"
#include "fs.h"
#include "log.h"
#include "lang.h"

#include "game_server.h"
#include "game_client.h"
#include "game_proxy.h"

/*---------------------------------------------------------------------------*/

struct set
{
    char file[PATHMAX];

    char *id;                           /* Internal set identifier           */
    char *name;                         /* Set name                          */
    char *desc;                         /* Set description                   */
    char *shot;                         /* Set screen-shot                   */

    char *user_scores;                  /* User high-score file              */
    char *cheat_scores;                 /* Cheat mode score file             */

    struct score coin_score;            /* Challenge score                   */
    struct score time_score;            /* Challenge score                   */

    /* Level info                                                            */

    int   count;                        /* Number of levels                  */
    char *level_name_v[MAXLVL];         /* List of level file names          */
};

#define SET_GET(a, i) ((struct set *) array_get((a), (i)))

static Array sets;
static int   curr;

static struct level level_v[MAXLVL];

/*---------------------------------------------------------------------------*/

#define SCORE_VERSION 2

static int score_version;

static void put_score(fs_file fp, const struct score *s)
{
    int i;

    for (i = RANK_HARD; i <= RANK_EASY; i++)
        fs_printf(fp, "%d %d %s\n", s->timer[i], s->coins[i], s->player[i]);
}

static int get_score(fs_file fp, struct score *s)
{
    char line[MAXSTR];
    int i;

    for (i = RANK_HARD; i <= RANK_EASY; i++)
    {
        int n = -1;

        if (!fs_gets(line, sizeof (line), fp))
            return 0;

        strip_newline(line);

        if (sscanf(line, "%d %d %n", &s->timer[i], &s->coins[i], &n) < 2)
            return 0;

        if (n < 0)
            return 0;

        SAFECPY(s->player[i], line + n);
    }

    return 1;
}

void set_store_hs(void)
{
    const struct set *s = SET_GET(sets, curr);
    fs_file fp;

    if ((fp = fs_open_write(config_cheat() ? s->cheat_scores : s->user_scores)))
    {
        int i;

        fs_printf(fp, "version %d\nset %s\n", SCORE_VERSION, s->id);

        put_score(fp, &s->time_score);
        put_score(fp, &s->coin_score);

        for (i = 0; i < s->count; i++)
        {
            const struct level *l = &level_v[i];

            int flags = 0;

            if (l->is_locked)    flags |= LEVEL_LOCKED;
            if (l->is_completed) flags |= LEVEL_COMPLETED;

            fs_printf(fp, "level %d %d %s\n", flags, l->version_num, l->file);

            fs_printf(fp, "stats %d %d %d\n", l->stats.completed,
                      l->stats.timeout, l->stats.fallout);

            put_score(fp, &l->scores[SCORE_TIME]);
            put_score(fp, &l->scores[SCORE_GOAL]);
            put_score(fp, &l->scores[SCORE_COIN]);
        }

        fs_close(fp);
    }
}

static void set_load_hs_v1(fs_file fp, struct set *s, char *buf, int size)
{
    struct level *l;
    int i, n;

    /* First line holds level states. */

    n = MIN(strlen(buf), s->count);

    for (i = 0; i < n; i++)
    {
        l = &level_v[i];

        l->is_locked    = (buf[i] == 'L');
        l->is_completed = (buf[i] == 'C');
    }

    get_score(fp, &s->time_score);
    get_score(fp, &s->coin_score);

    for (i = 0; i < n; i++)
    {
        l = &level_v[i];

        get_score(fp, &l->scores[SCORE_TIME]);
        get_score(fp, &l->scores[SCORE_GOAL]);
        get_score(fp, &l->scores[SCORE_COIN]);
    }
}

static struct level *find_level(const struct set *s, const char *file)
{
    int i;

    for (i = 0; i < s->count; i++)
        if (strcmp(level_v[i].file, file) == 0)
            return &level_v[i];

    return NULL;
}

static void get_stats(fs_file fp, struct level *l)
{
    char line[MAXSTR];

    if (!fs_gets(line, sizeof(line), fp))
        return;

    strip_newline(line);

    if (sscanf(line, "stats %d %d %d", &l->stats.completed,
                                        &l->stats.timeout,
                                        &l->stats.fallout) < 3) {
        /* compatible with save files without stats info */
        l->stats.completed = 0;
        l->stats.timeout   = 0;
        l->stats.fallout   = 0;

        /* stats not available, rewind file pointer */
        fs_seek(fp, - strlen(line) - 1, SEEK_CUR);
    }
}

static void set_load_hs_v2(fs_file fp, struct set *s, char *buf, int size)
{
    struct score time_score;
    struct score coin_score;

    int set_score = 0;
    int set_match = 1;

    while (fs_gets(buf, size, fp))
    {
        int version = 0;
        int flags = 0;
        int n = 0;

        strip_newline(buf);

        if (strncmp(buf, "set ", 4) == 0)
        {
            get_score(fp, &time_score);
            get_score(fp, &coin_score);

            set_score = 1;
        }
        else if (sscanf(buf, "level %d %d %n", &flags, &version, &n) >= 2)
        {
            struct level *l;

            if ((l = find_level(s, buf + n)))
            {
                get_stats(fp, l);
                /* Always prefer "locked" flag from the score file. */

                l->is_locked = !!(flags & LEVEL_LOCKED);

                /* Only use "completed" flag and scores on version match. */

                if (version == l->version_num)
                {
                    l->is_completed = !!(flags & LEVEL_COMPLETED);

                    get_score(fp, &l->scores[SCORE_TIME]);
                    get_score(fp, &l->scores[SCORE_GOAL]);
                    get_score(fp, &l->scores[SCORE_COIN]);
                }
                else set_match = 0;
            }
            else set_match = 0;
        }
    }

    if (set_match && set_score)
    {
        s->time_score = time_score;
        s->coin_score = coin_score;
    }
}

static void set_load_hs(void)
{
    struct set *s = SET_GET(sets, curr);
    fs_file fp;

    if ((fp = fs_open_read(config_cheat() ? s->cheat_scores : s->user_scores)))
    {
        char buf[MAXSTR];

        if (fs_gets(buf, sizeof (buf), fp))
        {
            strip_newline(buf);

            if (sscanf(buf, "version %d", &score_version) == 1)
            {
                switch (score_version)
                {
                case 2: set_load_hs_v2(fp, s, buf, sizeof (buf)); break;
                }
            }
            else
                set_load_hs_v1(fp, s, buf, sizeof (buf));
        }

        fs_close(fp);
    }
}

/*---------------------------------------------------------------------------*/

static int set_load(struct set *s, const char *filename)
{
    fs_file fin;
    char *scores, *level_name;

    /* Skip "Misc" set when not in dev mode. */

    if (strcmp(filename, SET_MISC) == 0 && !config_cheat())
        return 0;

    fin = fs_open_read(filename);

    if (!fin)
    {
        log_printf("Failure to load set file %s\n", filename);
        return 0;
    }

    memset(s, 0, sizeof (struct set));

    /* Set some sane values in case the scores are missing. */

    score_init_hs(&s->time_score, 359999, 0);
    score_init_hs(&s->coin_score, 359999, 0);

    SAFECPY(s->file, filename);

    if (read_line(&s->name, fin) &&
        read_line(&s->desc, fin) &&
        read_line(&s->id,   fin) &&
        read_line(&s->shot, fin) &&
        read_line(&scores,  fin))
    {
        sscanf(scores, "%d %d %d %d %d %d",
               &s->time_score.timer[RANK_HARD],
               &s->time_score.timer[RANK_MEDM],
               &s->time_score.timer[RANK_EASY],
               &s->coin_score.coins[RANK_HARD],
               &s->coin_score.coins[RANK_MEDM],
               &s->coin_score.coins[RANK_EASY]);

        free(scores);

        s->user_scores  = concat_string("Scores/", s->id, ".txt",       NULL);
        s->cheat_scores = concat_string("Scores/", s->id, "-cheat.txt", NULL);

        s->count = 0;

        while (s->count < MAXLVL && read_line(&level_name, fin))
        {
            strip_spaces(level_name);

            if (*level_name)
            {
                s->level_name_v[s->count] = level_name;
                s->count++;
            }
            else
            {
                free(level_name);
                level_name = NULL;
            }
        }

        fs_close(fin);

        return 1;
    }

    log_printf("Failure to load set file %s\n", filename);

    free(s->name);
    free(s->desc);
    free(s->id);
    free(s->shot);

    fs_close(fin);

    return 0;
}

static void set_free(struct set *s)
{
    int i;

    free(s->name);
    free(s->desc);
    free(s->id);
    free(s->shot);

    free(s->user_scores);
    free(s->cheat_scores);

    for (i = 0; i < s->count; i++)
        free(s->level_name_v[i]);
}

/*---------------------------------------------------------------------------*/

static int cmp_dir_items(const void *A, const void *B)
{
    const struct dir_item *a = A, *b = B;
    return strcmp(a->path, b->path);
}

static int set_is_loaded(const char *path)
{
    return (set_find(path) >= 0);
}

static int is_unseen_set(struct dir_item *item)
{
    return (str_starts_with(base_name(item->path), "set-") &&
            str_ends_with(item->path, ".txt") &&
            !set_is_loaded(item->path));
}

int set_init(void)
{
    fs_file fin;
    char *name;

    Array items;
    int i;

    if (sets)
        set_quit();

    sets = array_new(sizeof (struct set));
    curr = 0;

    /*
     * First, load the sets listed in the set file, preserving order.
     */

    if ((fin = fs_open_read(SET_FILE)))
    {
        while (read_line(&name, fin))
        {
            struct set *s = array_add(sets);

            if (!set_load(s, name))
                array_del(sets);

            free(name);
        }
        fs_close(fin);
    }

    /*
     * Then, scan for any remaining set description files, and add
     * them after the first group in alphabetic order.
     */

    if ((items = fs_dir_scan("", is_unseen_set)))
    {
        array_sort(items, cmp_dir_items);

        for (i = 0; i < array_len(items); i++)
        {
            struct set *s = array_add(sets);

            if (!set_load(s, DIR_ITEM_GET(items, i)->path))
                array_del(sets);
        }

        fs_dir_free(items);
    }

    return array_len(sets);
}

void set_quit(void)
{
    if (sets)
    {
        int i, n = array_len(sets);

        for (i = 0; i < n; i++)
            set_free(array_get(sets, i));

        array_free(sets);
        sets = NULL;
    }
}

/*---------------------------------------------------------------------------*/

int set_exists(int i)
{
    return sets ? 0 <= i && i < array_len(sets) : 0;
}

const char *set_file(int i)
{
    return set_exists(i) ? SET_GET(sets, i)->file : NULL;
}

const char *set_id(int i)
{
    return set_exists(i) ? SET_GET(sets, i)->id : NULL;
}

const char *set_name(int i)
{
    return set_exists(i) ? _(SET_GET(sets, i)->name) : NULL;
}

const char *set_desc(int i)
{
    return set_exists(i) ? _(SET_GET(sets, i)->desc) : NULL;
}

const char *set_shot(int i)
{
    return set_exists(i) ? SET_GET(sets, i)->shot : NULL;
}

const struct score *set_score(int i, int s)
{
    if (set_exists(i))
    {
        if (s == SCORE_TIME) return &SET_GET(sets, i)->time_score;
        if (s == SCORE_COIN) return &SET_GET(sets, i)->coin_score;
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/

static void set_load_levels(void)
{
    static const char *roman[] = {
        "",
        "I", "II", "III", "IV", "V",
        "VI", "VII", "VIII", "IX", "X",
        "XI", "XII", "XIII", "XIV", "XV",
        "XVI", "XVII", "XVIII", "XIX", "XX",
        "XXI", "XXII", "XXIII", "XXIV", "XXV"
    };

    struct set *s = SET_GET(sets, curr);
    int regular = 1, bonus = 1;
    int i;

    for (i = 0; i < s->count; i++)
    {
        struct level *l = &level_v[i];

        level_load(s->level_name_v[i], l);

        l->number = i;

        if (l->is_bonus)
        {
            SAFECPY(l->name, roman[bonus]);
            bonus++;
        }
        else
        {
            sprintf(l->name, "%02d", regular);
            regular++;
        }

        l->is_locked = (i > 0);
        l->is_completed = 0;

        if (i > 0)
            level_v[i - 1].next = l;
    }
}

void set_goto(int i)
{
    curr = i;

    set_load_levels();
    set_load_hs();
}

int set_find(const char *file)
{
    if (sets)
    {
        int i, n;

        for (i = 0, n = array_len(sets); i < n; ++i)
            if (strcmp(SET_GET(sets, i)->file, file) == 0)
                return i;
    }

    return -1;
}

/*
 * Find a level in the set given a SOL basename.
 */
struct level *set_find_level(const char *basename)
{
    if (sets && curr_set() < array_len(sets))
    {
        struct set *s = SET_GET(sets, curr_set());

        int i;

        for (i = 0; i < s->count; ++i)
        {
            if (strcmp(basename, base_name(level_v[i].file)) == 0)
                return &level_v[i];
        }
    }

    return NULL;
}

int curr_set(void)
{
    return curr;
}

struct level *get_level(int i)
{
    return (i >= 0 && i < SET_GET(sets, curr)->count) ? &level_v[i] : NULL;
}

/*---------------------------------------------------------------------------*/

int set_score_update(int timer, int coins, int *score_rank, int *times_rank)
{
    struct set *s = SET_GET(sets, curr);
    const char *player = config_get_s(CONFIG_PLAYER);

    score_coin_insert(&s->coin_score, score_rank, player, timer, coins);
    score_time_insert(&s->time_score, times_rank, player, timer, coins);

    if ((score_rank && *score_rank < RANK_LAST) ||
        (times_rank && *times_rank < RANK_LAST))
        return 1;
    else
        return 0;
}

void set_rename_player(int score_rank, int times_rank, const char *player)
{
    struct set *s = SET_GET(sets, curr);

    SAFECPY(s->coin_score.player[score_rank], player);
    SAFECPY(s->time_score.player[times_rank], player);
}

/*---------------------------------------------------------------------------*/

void level_snap(int i, const char *path)
{
    char *filename;

    /* Convert the level name to a PNG filename. */

    filename = concat_string(path,
                             "/",
                             base_name_sans(level_v[i].file, ".sol"),
                             ".png",
                             NULL);

    /* Initialize the game for a snapshot. */

    if (game_client_init(level_v[i].file))
    {
        union cmd cmd;
        cmd.type = CMD_GOAL_OPEN;
        game_proxy_enq(&cmd);
        game_client_sync(NULL);

        /* Render the level and grab the screen. */

        video_clear();
        game_client_fly(1.0f);
        game_kill_fade();
        game_client_draw(POSE_LEVEL, 0);
        video_snap(filename);
        video_swap();
    }

    free(filename);
}

void set_cheat(void)
{
    int i;

    for (i = 0; i < SET_GET(sets, curr)->count; i++)
    {
        level_v[i].is_locked    = 0;
        level_v[i].is_completed = 1;
    }
}

/*---------------------------------------------------------------------------*/
