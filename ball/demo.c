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

#ifndef _WIN32
#include <unistd.h>
#endif

#include "set.h"
#include "demo.h"
#include "game.h"
#include "level.h"
#include "audio.h"
#include "solid.h"
#include "config.h"

/*---------------------------------------------------------------------------*/

#define MAGIC 0x4E425250

struct demo_head
{
    int magic;

    char shot[PATHMAX];
    char file[PATHMAX];
    char back[PATHMAX];
    char grad[PATHMAX];
    char song[PATHMAX];

    int clock;
    int goal;
    int score;
    int coins;
    int balls;

    int total_coins;
    int total_clock;
};

static FILE *demo_fp;

static char             name[MAXDEMO][MAXNAM];
static struct demo_head head[MAXDEMO];

static int count;

/*---------------------------------------------------------------------------*/
#ifdef _WIN32

int demo_scan(void)
{
    WIN32_FIND_DATA d;
    HANDLE h;
    FILE *fp;

    count = 0;

    /* Scan the user directory for files. */

    if ((h = FindFirstFile(config_user("*"), &d)) != INVALID_HANDLE_VALUE)
    {
        do
            if ((fp = fopen(config_user(d.cFileName), FMODE_RB)))
            {
                /* Note the name and screen shot of each replay file. */

                if (fread(head + count, 1, sizeof (struct demo_head), fp))
                    if (head[count].magic == MAGIC)
                    {
                        strncpy(name[count], d.cFileName, MAXNAM);
                        count++;
                    }
                fclose(fp);
            }
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
    FILE *fp;
    DIR  *dp;

    count = 0;

    /* Scan the user directory for files. */

    if ((dp = opendir(config_user(""))))
    {
        while (count < MAXDEMO && (ent = readdir(dp)))
            if ((fp = fopen(config_user(ent->d_name), FMODE_RB)))
            {
                /* Note the name and screen shot of each replay file. */

                if (fread(head + count, 1, sizeof (struct demo_head), fp))
                    if (head[count].magic == MAGIC)
                    {
                        strncpy(name[count], ent->d_name, MAXNAM);
                        count++;
                    }
                fclose(fp);
            }

        closedir(dp);
    }
    return count;
}
#endif /* _WIN32 */

const char *demo_pick(void)
{
    int n = demo_scan();

    return (n > 0) ? name[(rand() >> 4) % n] : NULL;
}

const char *demo_name(int i)
{
    return (0 <= i && i < count) ? name[i] : NULL;
}

const char *demo_shot(int i)
{
    return (0 <= i && i < count) ? head[i].shot : NULL;
}

int demo_coins(int i)
{
    return (0 <= i && i < count) ? head[i].total_coins : 0;
}

int demo_clock(int i)
{
    return (0 <= i && i < count) ? head[i].total_clock : 0;
}

/*---------------------------------------------------------------------------*/

void demo_unique(char *name)
{
    FILE *fp;
    int i;

    /* Generate a unique name for a new replay save. */

    for (i = 1; i < 100; i++)
    {
        sprintf(name, "replay%02d", i);

        if ((fp = fopen(config_user(name), "r")) == NULL)
            return;

        fclose(fp);
    }
}

/*---------------------------------------------------------------------------*/

static struct demo_head demo_play_head;

int demo_play_init(const char *name,
                   const char *file,
                   const char *back,
                   const char *grad,
                   const char *song,
                   const char *shot,
                   int t, int g, int s, int c, int b)
{
    /* Initialize the replay file.  Write the header. */

    if (name && (demo_fp = fopen(config_user(name), FMODE_WB)))
    {
        demo_play_head.magic = MAGIC;

        strncpy(demo_play_head.shot, shot, PATHMAX);
        strncpy(demo_play_head.file, file, PATHMAX);
        strncpy(demo_play_head.back, back, PATHMAX);
        strncpy(demo_play_head.grad, grad, PATHMAX);
        strncpy(demo_play_head.song, song, PATHMAX);

        demo_play_head.clock  = t;
        demo_play_head.goal   = g;
        demo_play_head.score  = s;
        demo_play_head.coins  = c;
        demo_play_head.balls  = b;

        demo_play_head.total_coins = 0;
        demo_play_head.total_clock = 0;

        fwrite(&demo_play_head, 1, sizeof (struct demo_head), demo_fp);

        audio_music_fade_to(2.0f, song);

        return game_init(file, back, grad, t, (g == 0));
    }
    return 0;
}

void demo_play_step(float dt)
{
    if (demo_fp)
    {
        float_put(demo_fp, &dt);
        game_put(demo_fp);
    }
}

void demo_play_stat(int coins, int clock)
{
    if (demo_fp)
    {
        /* Update the demo header using the final score and time. */

        demo_play_head.total_coins = coins;
        demo_play_head.total_clock = clock;

        fseek(demo_fp, 0, SEEK_SET);
        fwrite(&demo_play_head, 1, sizeof (struct demo_head), demo_fp);
        fseek(demo_fp, 0, SEEK_END);
    }
}

void demo_play_stop(const char *name)
{
    char src[PATHMAX];
    char dst[PATHMAX];

    if (demo_fp)
    {
        fclose(demo_fp);
        demo_fp = NULL;

        /* Rename the temporary replay file to its given name. */

        if (name)
        {
            strncpy(src, config_user(USER_REPLAY_FILE), PATHMAX);
            strncpy(dst, config_user(name),             PATHMAX);

            rename(src, dst);
        }
    }
}

/*---------------------------------------------------------------------------*/

static char             demo_replay_name[MAXNAM];
static struct demo_head demo_replay_head;

int demo_replay_init(const char *name, int *s, int *c, int *b, int *g)
{
    if ((demo_fp = fopen(config_user(name), FMODE_RB)))
    {
        fread(&demo_replay_head, 1, sizeof (struct demo_head), demo_fp);

        if (demo_replay_head.magic == MAGIC)
        {
            strncpy(demo_replay_name, name, MAXNAM);

            if (s) *s = demo_replay_head.score;
            if (c) *c = demo_replay_head.coins;
            if (b) *b = demo_replay_head.balls;
            if (g) *g = demo_replay_head.goal;

            if (g)
            {
                audio_music_fade_to(0.5f, demo_replay_head.song);
                return game_init(demo_replay_head.file,
                                 demo_replay_head.back,
                                 demo_replay_head.grad,
                                 demo_replay_head.clock, (*g == 0));
            }
            else
            {
                return game_init(demo_replay_head.file,
                                 demo_replay_head.back,
                                 demo_replay_head.grad,
                                 demo_replay_head.clock, 1);
            }
        }
        fclose(demo_fp);
    }
    return 0;
}

int demo_replay_step(float *dt)
{
    const float g[3] = { 0.0f, -9.8f, 0.0f };

    if (demo_fp && float_get(demo_fp, dt))
    {
        /* Play out current game state for particles, clock, etc. */

        game_step(g, *dt, 1);

        /* Load real curren game stat from file. */

        if (game_get(demo_fp))
            return 1;
    }
    return 0;
}

void demo_replay_stop(int d)
{
    if (demo_fp)
    {
        fclose(demo_fp);
        demo_fp = NULL;

        if (d) unlink(config_user(demo_replay_name));
    }
}

/*---------------------------------------------------------------------------*/
