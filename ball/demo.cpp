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
#include <assert.h>

#include "demo.h"
#include "audio.h"
#include "config.h"
#include "binary.h"
#include "common.h"
#include "level.h"
#include "array.h"
#include "dir.h"

#include "game_server.h"
#include "game_client.h"
#include "game_proxy.h"
#include "game_common.h"

#define DEMO_MAGIC (0xAF | 'N' << 8 | 'B' << 16 | 'R' << 24)
#define DEMO_VERSION 9

#define DATELEN sizeof ("YYYY-MM-DDTHH:MM:SS")

fs_file demo_fp;

/*---------------------------------------------------------------------------*/

static const char *demo_path(const char *name)
{
    static char path[MAXSTR];
    sprintf(path, "Replays/%s.nbr", name);
    return path;
}

static const char *demo_name(const char *path)
{
    static char name[MAXSTR];
    SAFECPY(name, base_name_sans(path, ".nbr"));
    return name;
}

/*---------------------------------------------------------------------------*/

static int demo_header_read(fs_file fp, struct demo *d)
{
    int magic;
    int version;
    int t;

    struct tm date;
    char datestr[DATELEN];

    magic   = get_index(fp);
    version = get_index(fp);

    t = get_index(fp);

    if (magic == DEMO_MAGIC && version == DEMO_VERSION && t)
    {
        d->timer = t;

        d->coins  = get_index(fp);
        d->status = get_index(fp);
        d->mode   = get_index(fp);

        get_string(fp, d->player, sizeof (d->player));
        get_string(fp, datestr, sizeof (datestr));

        sscanf(datestr,
               "%d-%d-%dT%d:%d:%d",
               &date.tm_year,
               &date.tm_mon,
               &date.tm_mday,
               &date.tm_hour,
               &date.tm_min,
               &date.tm_sec);

        date.tm_year -= 1900;
        date.tm_mon  -= 1;
        date.tm_isdst = -1;

        d->date = make_time_from_utc(&date);

        get_string(fp, d->shot, PATHMAX);
        get_string(fp, d->file, PATHMAX);

        d->time  = get_index(fp);
        d->goal  = get_index(fp);
        (void)     get_index(fp);
        d->score = get_index(fp);
        d->balls = get_index(fp);
        d->times = get_index(fp);

        return 1;
    }
    return 0;
}

static void demo_header_write(fs_file fp, struct demo *d)
{
    char datestr[DATELEN];

    strftime(datestr, sizeof (datestr), "%Y-%m-%dT%H:%M:%S", gmtime(&d->date));

    put_index(fp, DEMO_MAGIC);
    put_index(fp, DEMO_VERSION);
    put_index(fp, 0);
    put_index(fp, 0);
    put_index(fp, 0);
    put_index(fp, d->mode);

    put_string(fp, d->player);
    put_string(fp, datestr);

    put_string(fp, d->shot);
    put_string(fp, d->file);

    put_index(fp, d->time);
    put_index(fp, d->goal);
    put_index(fp, 0);                   /* Unused (was goal enabled flag).   */
    put_index(fp, d->score);
    put_index(fp, d->balls);
    put_index(fp, d->times);
}

/*---------------------------------------------------------------------------*/

int demo_load(struct demo *d, const char *path)
{
    int rc = 0;

    if (d)
    {
        fs_file fp;

        memset(d, 0, sizeof (*d));

        if ((fp = fs_open(path, "r")))
        {
            SAFECPY(d->path, path);
            SAFECPY(d->name, demo_name(path));

            if (demo_header_read(fp, d))
                rc = 1;

            fs_close(fp);
        }
    }

    return rc;
}

void demo_free(struct demo *d)
{
}

/*---------------------------------------------------------------------------*/

int demo_exists(const char *name)
{
    return fs_exists(demo_path(name));
}

const char *demo_format_name(const char *fmt,
                             const char *set,
                             const char *level)
{
    static char name[MAXSTR];
    int space_left;
    char *numpart;
    int i;

    if (!fmt)
        return NULL;

    if (!set)
        set = "none";

    if (!level)
        level = "00";

    memset(name, 0, sizeof (name));
    space_left = MAXSTRLEN(name);

    /* Construct name, replacing each format sequence as appropriate. */

    while (*fmt && space_left > 0)
    {
        if (*fmt == '%')
        {
            fmt++;

            switch (*fmt)
            {
            case 's':
                strncat(name, set, space_left);
                space_left -= strlen(set);
                break;

            case 'l':
                strncat(name, level, space_left);
                space_left -= strlen(level);
                break;

            case '%':
                strncat(name, "%", space_left);
                space_left--;
                break;

            case '\0':
                /* Missing format. */
                fmt--;
                break;

            default:
                /* Invalid format. */
                break;
            }
        }
        else
        {
            strncat(name, fmt, 1);
            space_left--;
        }

        fmt++;
    }

    /*
     * Append a unique 2-digit number preceded by an underscore to the
     * file name, discarding characters if there's not enough space
     * left in the buffer.
     */

    if (space_left < strlen("_23"))
        numpart = name + MAXSTRLEN(name) - strlen("_23");
    else
        numpart = name + MAXSTRLEN(name) - space_left;

    for (i = 1; i < 100; i++)
    {
        sprintf(numpart, "_%02d", i);

        if (!demo_exists(name))
            break;
    }

    return name;
}

/*---------------------------------------------------------------------------*/

static struct demo demo_play;

int demo_play_init(const char *name, const struct level *level,
                   int mode, int scores, int balls, int times)
{
    struct demo *d = &demo_play;

    memset(d, 0, sizeof (*d));

    SAFECPY(d->path,   demo_path(name));
    SAFECPY(d->name,   name);
    SAFECPY(d->player, config_get_s(CONFIG_PLAYER));
    SAFECPY(d->shot,   level_shot(level));
    SAFECPY(d->file,   level_file(level));

    d->mode  = mode;
    d->date  = time(NULL);
    d->time  = level_time(level);
    d->goal  = level_goal(level);
    d->score = scores;
    d->balls = balls;
    d->times = times;

    if ((demo_fp = fs_open(d->path, "w")))
    {
        demo_header_write(demo_fp, d);
        return 1;
    }
    return 0;
}

void demo_play_stat(int status, int coins, int timer)
{
    if (demo_fp)
    {
        long pos = fs_tell(demo_fp);

        fs_seek(demo_fp, 8, SEEK_SET);

        put_index(demo_fp, timer);
        put_index(demo_fp, coins);
        put_index(demo_fp, status);

        fs_seek(demo_fp, pos, SEEK_SET);
    }
}

void demo_play_stop(int d)
{
    if (demo_fp)
    {
        fs_close(demo_fp);
        demo_fp = NULL;

        if (d) fs_remove(demo_play.path);
    }
}

int demo_saved(void)
{
    return fs_exists(demo_play.path);
}

void demo_rename(const char *name)
{
    char path[MAXSTR];

    if (name && *name)
    {
        SAFECPY(path, demo_path(name));

        if (strcmp(demo_play.name, name) != 0 && fs_exists(demo_play.path))
            fs_rename(demo_play.path, path);
    }
}

void demo_rename_player(const char *name, const char *player)
{
    /* TODO */
    return;
}

/*---------------------------------------------------------------------------*/

extern struct lockstep update_step;

static void demo_update_read(float dt)
{
    if (demo_fp)
    {
        union cmd cmd;

        while (cmd_get(demo_fp, &cmd))
        {
            game_proxy_enq(&cmd);

            if (cmd.type == CMD_UPDATES_PER_SECOND)
                update_step.dt = 1.0f / cmd.ups.n;

            if (cmd.type == CMD_END_OF_UPDATE)
            {
                game_client_sync(NULL);
                break;
            }
        }

    }
}

static struct lockstep update_step = { demo_update_read, DT };

float demo_replay_blend(void)
{
    return lockstep_blend(&update_step);
}

/*---------------------------------------------------------------------------*/

static struct demo demo_replay;

const char *curr_demo(void)
{
    return demo_replay.path;
}

int demo_replay_init(const char *path, int *g, int *m, int *b, int *s, int *tt)
{
    lockstep_clr(&update_step);

    if ((demo_fp = fs_open(path, "r")))
    {
        if (demo_header_read(demo_fp, &demo_replay))
        {
            struct level level;

            SAFECPY(demo_replay.path, path);
            SAFECPY(demo_replay.name, demo_name(path));

            if (level_load(demo_replay.file, &level))
            {
                if (g)  *g  = demo_replay.goal;
                if (m)  *m  = demo_replay.mode;
                if (b)  *b  = demo_replay.balls;
                if (s)  *s  = demo_replay.score;
                if (tt) *tt = demo_replay.times;

                /*
                 * Init client and then read and process the first batch of
                 * commands from the replay file.
                 */

                if (game_client_init(demo_replay.file))
                {
                    if (g)
                    {
                        audio_music_fade_to(0.5f, level.song);
                    }
                    else
                    {
                        union cmd cmd;
                        cmd.type = CMD_GOAL_OPEN;
                        game_proxy_enq(&cmd);
                    }

                    demo_update_read(0);

                    if (!fs_eof(demo_fp))
                        return 1;
                }
            }
        }

        fs_close(demo_fp);
        demo_fp = NULL;
    }

    return 0;
}

int demo_replay_step(float dt)
{
    if (demo_fp)
    {
        lockstep_run(&update_step, dt);
        return !fs_eof(demo_fp);
    }
    return 0;
}

void demo_replay_stop(int d)
{
    if (demo_fp)
    {
        fs_close(demo_fp);
        demo_fp = NULL;

        if (d) fs_remove(demo_replay.path);
    }
}

void demo_replay_speed(int speed)
{
    if (SPEED_NONE <= speed && speed < SPEED_MAX)
        lockstep_scl(&update_step, SPEED_FACTORS[speed]);
}

/*---------------------------------------------------------------------------*/
