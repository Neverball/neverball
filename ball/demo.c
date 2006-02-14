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
#include "binary.h"

/*---------------------------------------------------------------------------*/

#define MAGIC 0x4E425251
#define DEMO_FPS_CAP 200 /* FPS replay limit, keeps size down on monster systems */

static FILE *demo_fp;

static char name[MAXDEMO][MAXNAM];
static char shot[MAXDEMO][PATHMAX];
static int  score[MAXDEMO];
static int  timer[MAXDEMO];
static int  count;

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
                int magic;
                int t, c;

                /* Note the name and screen shot of each replay file. */

                get_index(fp, &magic);
                get_index(fp, &t);
                get_index(fp, &c);

                if (magic == MAGIC && t)
                {
                    fread(shot[count], 1, PATHMAX, fp);
                    timer[count] = t;
                    score[count] = c;
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
                int magic;
                int t, c;

                /* Note the name and screen shot of each replay file. */

                get_index(fp, &magic);
                get_index(fp, &t);
                get_index(fp, &c);

                if (magic == MAGIC && t)
                {
                    fread(shot[count], 1, PATHMAX, fp);
                    timer[count] = t;
                    score[count] = c;
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
    return (0 <= i && i < count) ? shot[i] : NULL;
}

int demo_coins(int i)
{
    return (0 <= i && i < count) ? score[i] : 0;
}

int demo_clock(int i)
{
    return (0 <= i && i < count) ? timer[i] : 0;
}

/*---------------------------------------------------------------------------*/

int demo_exists(char *name)
{
    FILE *fp;

    if ((fp = fopen(config_user(name), "r")))
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
        sprintf(name, _("replay%02d"), i);

        if (!demo_exists(name))
            return;
    }
}

/*---------------------------------------------------------------------------*/

int demo_play_init(const char *name,
                   const char *file,
                   const char *back,
                   const char *grad,
                   const char *song,
                   const char *shot,
                   int t, int g, int s, int c, int b)
{
    int magic = MAGIC;
    int zero  = 0;
    int st    = t;
    int sg    = g;
    int ss    = s;
    int sc    = c;
    int sb    = b;

    /* Initialize the replay file.  Write the header. */

    if (name && (demo_fp = fopen(config_user(name), FMODE_WB)))
    {
        put_index(demo_fp, &magic);
        put_index(demo_fp, &zero);
        put_index(demo_fp, &zero);

        fwrite(shot, 1, PATHMAX, demo_fp);
        fwrite(file, 1, PATHMAX, demo_fp);
        fwrite(back, 1, PATHMAX, demo_fp);
        fwrite(grad, 1, PATHMAX, demo_fp);
        fwrite(song, 1, PATHMAX, demo_fp);

        put_index(demo_fp, &st);
        put_index(demo_fp, &sg);
        put_index(demo_fp, &ss);
        put_index(demo_fp, &sc);
        put_index(demo_fp, &sb);

        audio_music_fade_to(2.0f, song);

        return game_init(file, back, grad, t, (g == 0));
    }
    return 0;
}

void demo_play_step(float dt)
{
    static float fps_track = 0.0f;
    static float fps_cap   = 1.0f / (float) DEMO_FPS_CAP;

    if (demo_fp)
    {
        fps_track += dt;
        if (fps_track > fps_cap)
        {            
            put_float(demo_fp, &fps_track);
            put_game_state(demo_fp); 
            fps_track = 0.0f;
        }
    }
}

void demo_play_stat(int coins, int timer)
{
    int c = coins;
    int t = timer;

    if (demo_fp)
    {
        /* Update the demo header using the final score and time. */

        fseek(demo_fp, 4, SEEK_SET);

        put_index(demo_fp, &t);
        put_index(demo_fp, &c);

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

static char demo_replay_name[MAXSTR];

int demo_replay_init(const char *name, int *s, int *c, int *b, int *g)
{
    char shot[PATHMAX];
    char file[PATHMAX];
    char back[PATHMAX];
    char grad[PATHMAX];
    char song[PATHMAX];

    int magic;
    int zero;
    int st;
    int sg;
    int ss;
    int sc;
    int sb;
    
    if ((demo_fp = fopen(config_user(name), FMODE_RB)))
    {
        strncpy(demo_replay_name, name, MAXSTR);

        get_index(demo_fp, &magic);

        if (magic == MAGIC)
        {
            get_index(demo_fp, &zero);
            get_index(demo_fp, &zero);

            fread(shot, 1, PATHMAX, demo_fp);
            fread(file, 1, PATHMAX, demo_fp);
            fread(back, 1, PATHMAX, demo_fp);
            fread(grad, 1, PATHMAX, demo_fp);
            fread(song, 1, PATHMAX, demo_fp);

            get_index(demo_fp, &st);
            get_index(demo_fp, &sg);
            get_index(demo_fp, &ss);
            get_index(demo_fp, &sc);
            get_index(demo_fp, &sb);

            if (g) *g = (int) sg;
            if (s) *s = (int) ss;
            if (c) *c = (int) sc;
            if (b) *b = (int) sb;

            if (g)
            {
                audio_music_fade_to(0.5f, song);
                return game_init(file, back, grad, st, (*g == 0));
            }
            else
                return game_init(file, back, grad, st, 1);
        }
        fclose(demo_fp);
    }
    return 0;
}

int demo_replay_step(float *dt)
{
    const float g[3] = { 0.0f, -9.8f, 0.0f };

    if (demo_fp)
    {
        get_float(demo_fp, dt);

        if (feof(demo_fp) == 0)
        {
            /* Play out current game state for particles, clock, etc. */

            game_step(g, *dt, 1);

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

        if (d) unlink(config_user(demo_replay_name));
    }
}

/*---------------------------------------------------------------------------*/
