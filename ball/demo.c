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
#include <time.h>

#include "demo.h"
#include "game.h"
#include "audio.h"
#include "solid.h"
#include "config.h"
#include "binary.h"

/*---------------------------------------------------------------------------*/

#define MAGIC           0x52424EAF
#define DEMO_VERSION    3

#define DATELEN 20         

static FILE *demo_fp;

static struct demo demos[MAXDEMO]; /* Array of scanned demos  */
static int         count;          /* Number of scanned demos */

/*---------------------------------------------------------------------------*/

void demo_dump_info(const struct demo *d)
{
    printf("Name:         %s\n"
           "File:         %s\n"
           "Time:         %d\n"
           "Coins:        %d\n"
           "Mode:         %d\n"
           "State:        %d\n"
           "Date:         %s"
           "Player:       %s\n"
           "Shot:         %s\n"
           "Level:        %s\n"
           "  Back:       %s\n"
           "  Grad:       %s\n"
           "  Song:       %s\n"
           "Time:         %d\n"
           "Goal:         %d\n"
           "Score:        %d\n"
           "Balls:        %d\n"
           "Total Time:   %d\n",
           d->name, d->filename,
           d->timer, d->coins, d->mode, d->state, ctime(&d->date),
           d->player,
           d->shot, d->file, d->back, d->grad, d->song,
           d->time, d->goal, d->score, d->balls, d->times);
}

static time_t make_time_from_utc(struct tm *tm)
{
    struct tm local, *utc;
    time_t t;

    t = mktime(tm);

    local = *localtime(&t);
    utc   =  gmtime(&t);

    local.tm_year += local.tm_year - utc->tm_year;
    local.tm_mon  += local.tm_mon  - utc->tm_mon ;
    local.tm_mday += local.tm_mday - utc->tm_mday;
    local.tm_hour += local.tm_hour - utc->tm_hour;
    local.tm_min  += local.tm_min  - utc->tm_min ;
    local.tm_sec  += local.tm_sec  - utc->tm_sec ;

    return mktime(&local);
}

static int demo_header_read(FILE *fp, struct demo *d)
{
    int magic;
    int version;
    int t;

    struct tm date;
    char datestr[DATELEN];

    get_index(fp, &magic);
    get_index(fp, &version);

    get_index(fp, &t);

    if (magic == MAGIC && version == DEMO_VERSION && t)
    {
        d->timer = t;

        get_index(fp, &d->coins);
        get_index(fp, &d->state);
        get_index(fp, &d->mode);

        fread(d->player, 1, MAXNAM, fp);

        fread(datestr, 1, DATELEN, fp);
        sscanf(datestr,
               "%d-%d-%dT%d:%d:%d",
               &date.tm_year,
               &date.tm_mon,
               &date.tm_mday,
               &date.tm_hour,
               &date.tm_min,
               &date.tm_sec);

        /* Convert certain values to valid structure member values. */

        date.tm_year -= 1900;
        date.tm_mon  -= 1;

        d->date = make_time_from_utc(&date);

        fread(d->shot, 1, PATHMAX, fp);
        fread(d->file, 1, PATHMAX, fp);

        get_index(fp, &d->time);
        get_index(fp, &d->goal);
        get_index(fp, &d->score);
        get_index(fp, &d->balls);
        get_index(fp, &d->times);

        return 1;
    }
    return 0;
}

static char *bname(const char *name, const char *suffix)
{
    static char buf[MAXSTR];

    char *base;
    size_t l;

    /* Remove the directory delimiter */

    base = strrchr(name, '/');
#ifdef _WIN32
    if (!base)
        base = strrchr(name, '\\');
    else
    {
        char *tmp;
        if ((tmp = strrchr(base, '\\')))
            base = tmp;
    }
#endif
    strncpy(buf, base ? base + 1 : name, MAXSTR);

    /* Remove the extension */

    l = strlen(buf) - strlen(suffix);
    if ((l > 1) && (strcmp(buf + l, suffix) == 0))
        buf[l] = '\0';

    return buf;
}

static void demo_header_write(FILE *fp, struct demo *d)
{
    int magic = MAGIC;
    int version = DEMO_VERSION;
    int zero  = 0;

    char datestr[DATELEN];

    strftime(datestr, DATELEN, "%Y-%m-%dT%H:%M:%S", gmtime(&d->date));

    put_index(fp, &magic);
    put_index(fp, &version);
    put_index(fp, &zero);
    put_index(fp, &zero);
    put_index(fp, &zero);
    put_index(fp, &d->mode);

    fwrite(d->player, 1, MAXNAM, fp);
    fwrite(datestr, 1, DATELEN, fp);

    fwrite(d->shot, 1, PATHMAX, fp);
    fwrite(d->file, 1, PATHMAX, fp);

    put_index(fp, &d->time);
    put_index(fp, &d->goal);
    put_index(fp, &d->score);
    put_index(fp, &d->balls);
    put_index(fp, &d->times);
}

/* Update the demo header using the final level state. */

void demo_header_stop(FILE *fp, int coins, int timer, int state)
{
    long pos = ftell(fp);

    fseek(fp, 8, SEEK_SET);
    put_index(fp, &timer);
    put_index(fp, &coins);
    put_index(fp, &state);
    fseek(fp, pos, SEEK_SET);
}

/*---------------------------------------------------------------------------*/

/* Scan another file (used by demo_scan). */

static void demo_scan_file(const char *filename)
{
    FILE *fp;
    struct demo *d = &demos[count];

    if ((fp = fopen(config_user(filename), FMODE_RB)))
    {
        if (demo_header_read(fp, d))
        {
            strncpy(d->filename, config_user(filename),       MAXSTR);
            strncpy(d->name,     bname(filename, REPLAY_EXT), PATHMAX);
            d->name[PATHMAX - 1] = '\0';

            count++;
        }
        fclose(fp);
    }
}

#ifdef _WIN32

int demo_scan(void)
{
    WIN32_FIND_DATA d;
    HANDLE h;

    count = 0;

    /* Scan the user directory for files. */

    if ((h = FindFirstFile(config_user("*"), &d)) != INVALID_HANDLE_VALUE)
    {
        do
            demo_scan_file(d.cFileName);
        while (count < MAXDEMO && FindNextFile(h, &d));

        FindClose(h);
    }
    return count;
}

#else /* _WIN32 */
#include <dirent.h>

int demo_scan(void)
{
    struct dirent *ent;
    DIR  *dp;

    count = 0;

    /* Scan the user directory for files. */

    if ((dp = opendir(config_user(""))))
    {
        while (count < MAXDEMO && (ent = readdir(dp)))
            demo_scan_file(ent->d_name);

        closedir(dp);
    }
    return count;
}
#endif /* _WIN32 */

const char *demo_pick(void)
{
    int n = demo_scan();

    return (n > 0) ? demos[(rand() >> 4) % n].filename : NULL;
}

const struct demo *demo_get(int i)
{
    return (0 <= i && i < count) ? &demos[i] : NULL;
}

const char *date_to_str(time_t i)
{
    static char str[MAXSTR];

    /* TRANSLATORS:  here is the format of the date shown at the
       replay selection screen.  The default will work in most cases, so
       you should only change it if something's horribly wrong, like,
       for instance, the GUI layout is broken.  See strftime(3) for
       details on the format.
     */

    strftime(str, MAXSTR, /* xgettext:no-c-format */ _("%c"), localtime(&i));

    return str;
}

/*---------------------------------------------------------------------------*/

int demo_exists(const char *name)
{
    FILE *fp;
    char buf[MAXSTR];

    strcpy(buf, config_user(name));
    strcat(buf, REPLAY_EXT);
    if ((fp = fopen(buf, "r")))
    {
        fclose(fp);
        return 1;
    }
    return 0;
}

void demo_unique(char *name)
{
    int i;

    /* Generate a unique name for a new replay save. */

    for (i = 1; i < 100; i++)
    {
        sprintf(name, "replay%02d", i);

        if (!demo_exists(name))
            return;
    }
}

/*---------------------------------------------------------------------------*/

int demo_play_init(const char *name,
                   const struct level *level,
                   const struct level_game *lg)
{
    struct demo demo;

    memset(&demo, 0, sizeof (demo));

    strncpy(demo.filename, config_user(name), MAXSTR);
    strcat(demo.filename, REPLAY_EXT);

    demo.mode = lg->mode;
    demo.date = time(NULL);

    config_get_s(CONFIG_PLAYER, demo.player, MAXNAM);

    strncpy(demo.shot, level->shot, PATHMAX);
    strncpy(demo.file, level->file, PATHMAX);
    strncpy(demo.back, level->back, PATHMAX);
    strncpy(demo.grad, level->grad, PATHMAX);
    strncpy(demo.song, level->song, PATHMAX);

    demo.time  = lg->time;
    demo.goal  = lg->goal;
    demo.score = lg->score;
    demo.balls = lg->balls;
    demo.times = lg->times;

    if (demo.filename && (demo_fp = fopen(demo.filename, FMODE_WB)))
    {
        demo_header_write(demo_fp, &demo);
        audio_music_fade_to(2.0f, level->song);
        return game_init(level, lg->time, lg->goal);
    }
    return 0;
}

void demo_play_step(float dt)
{
    if (demo_fp)
    {
        put_float(demo_fp, &dt);
        put_game_state(demo_fp);
    }
}

/* Update the demo header using the final level state. */

void demo_play_stop(const struct level_game *lg)
{
    if (demo_fp)
    {
        demo_header_stop(demo_fp, lg->coins, lg->timer, lg->state);
        fclose(demo_fp);
        demo_fp = NULL;
    }
}

int demo_play_saved(void)
{
    return demo_exists(USER_REPLAY_FILE);
}

void demo_play_save(const char *name)
{
    char src[MAXSTR];
    char dst[MAXSTR];

    if (name &&
        demo_exists(USER_REPLAY_FILE) &&
        strcmp(name, USER_REPLAY_FILE) != 0)
    {
        strcpy(src, config_user(USER_REPLAY_FILE));
        strcat(src, REPLAY_EXT);
        strcpy(dst, config_user(name));
        strcat(dst, REPLAY_EXT);

#ifdef _WIN32
        if (demo_exists(name))
            remove(dst);
#endif
        rename(src, dst);
    }
}

/*---------------------------------------------------------------------------*/

static int demo_load_level(const struct demo *demo, struct level *level)
{
    if (level_load(demo->file, level))
    {
        level->time = demo->time;
        level->goal = demo->goal;
        return 1;
    }
    return 0;
}

static struct demo  demo_replay;       /* The current demo */
static struct level demo_level_replay; /* The current level demo-ed*/

const struct demo *curr_demo_replay(void)
{
    return &demo_replay;
}

/* Internally load a replay and fill the lg structure (if not NULL) */

int demo_replay_init(const char *name, struct level_game *lg)
{
    demo_fp = fopen(name, FMODE_RB);

    if (demo_fp && demo_header_read(demo_fp, &demo_replay))
    {
        strncpy(demo_replay.filename, name,                    MAXSTR);
        strncpy(demo_replay.name,     bname(name, REPLAY_EXT), PATHMAX);

        if (!demo_load_level(&demo_replay, &demo_level_replay))
            return 0;

        if (lg)
        {
            lg->mode = demo_replay.mode;
            lg->score = demo_replay.score;
            lg->times = demo_replay.times;
            lg->time = demo_replay.time;
            lg->goal = demo_replay.goal;

            /* A normal replay demo */
            audio_music_fade_to(0.5f, demo_level_replay.song);
            return game_init(&demo_level_replay, demo_replay.time,
                             demo_replay.goal);
        }
        else /* A title screen demo */
            return game_init(&demo_level_replay, demo_replay.time, 0);
    }
    return 0;
}

int demo_replay_step(float *dt)
{
    const float g[3] = { 0.0f, -9.8f, 0.0f };
    int sv;

    if (demo_fp)
    {
        get_float(demo_fp, dt);

        if (feof(demo_fp) == 0)
        {
            /* Play out current game state for particles, clock, etc. */

            game_step(g, *dt, &sv);

            /* Load real current game state from file. */

            if (get_game_state(demo_fp))
                return 1;
        }
    }
    return 0;
}

void demo_replay_stop(int d)
{
    if (demo_fp)
    {
        fclose(demo_fp);
        demo_fp = NULL;

        if (d) remove(demo_replay.filename);
    }
}

void demo_replay_dump_info(void)
{
    demo_dump_info(&demo_replay);
}

/*---------------------------------------------------------------------------*/
