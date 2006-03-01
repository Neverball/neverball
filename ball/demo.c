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

#define MAGIC 0x4E425252
#define DEMO_FPS_CAP 200 /* FPS replay limit, keeps size down on monster systems */

static FILE *demo_fp;


/* Demo information structure (header) */
struct demo
{
    char   name[MAXNAM];    /* demo filename */

    /* magic */
    int    timer;           /* elapsed time */
    int    coins;           /* coin number */
    int    state;           /* how the replay end */
    int    mode;            /* game mode */
    time_t date;            /* date of creation */
    char   player[MAXNAM];  /* player name */
    char   shot[PATHMAX];   /* image filename */
    char   file[PATHMAX];   /* level filename */
    char   back[PATHMAX];   /* level bg filename */
    char   grad[PATHMAX];   /* level gradiant filename */
    char   song[PATHMAX];   /* level song filename */
    int    time;            /* time limit (! training mode) */
    int    goal;            /* coin to open the goal (! training mode) */
    int    score;           /* sum of coins (challenge mode) */
    int    balls;           /* number of balls (challenge mode) */
    int    total_time;      /* total time (challenge mode) */
};


static struct demo demos[MAXDEMO]; /* Array of scanned demos */

static int count; /* number of scanned demos */

/*---------------------------------------------------------------------------*/

void demo_dump_info(struct demo * d)
/* This function dump the info of a demo structure*/
{
    printf("Filename: %s\n"
	   "Time:     %d\n"
	   "Coins:    %d\n"
	   "Mode:     %d\n"
	   "State:    %d\n"
	   "Date:     %s"
	   "Player:   %s\n"
	   "Shot:     %s\n"
	   "Level:    %s\n"
	   "  Back:   %s\n"
	   "  Grad:   %s\n"
	   "  Song:   %s\n"
	   "Time:     %d\n"
	   "Goal:     %d\n"
	   "Score:    %d\n"
	   "Balls:    %d\n"
	   "Tot Time: %d\n",
	   d->name,
	   d->timer, d->coins, d->mode, d->state, ctime(&d->date),
	   d->player,
	   d->shot, d->file, d->back, d->grad, d->song,
	   d->time, d->goal, d->score, d->balls, d->total_time);
}

static FILE * demo_header(const char * filename, struct demo * d)
/* Open a demo file, fill the demo information structure
If success, return the file pointer positioned after the header
If fail, return null */
{
    FILE *fp;
    if ((fp = fopen(config_user(filename), FMODE_RB)))
    {
	int magic;
	int t;

	get_index(fp, &magic);
	get_index(fp, &t);

	if (magic == MAGIC && t)
	{
	    strncpy(d->name, filename, MAXNAM);
	    d->timer = t;
	    get_index(fp, &d->coins);
	    get_index(fp, &d->state);
	    get_index(fp, &d->mode);
	    get_index(fp, (int*)&d->date);
	    fread(d->player, 1, MAXNAM,  fp);
	    fread(d->shot,   1, PATHMAX, fp);
            fread(d->file,   1, PATHMAX, fp);
            fread(d->back,   1, PATHMAX, fp);
            fread(d->grad,   1, PATHMAX, fp);
            fread(d->song,   1, PATHMAX, fp);
            get_index(fp, &d->time);
            get_index(fp, &d->goal);
            get_index(fp, &d->score);
            get_index(fp, &d->balls);
            get_index(fp, &d->total_time);

	    return fp;
	}
	fclose(fp);
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/

static void demo_scan_file(const char * filename)
/* Scan a other file (used by demo_scan */
{
    FILE *fp;
    if ((fp = demo_header(filename, &demos[count])))
    {
	count++;
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

    return (n > 0) ? demos[(rand() >> 4) % n].name : NULL;
}

const char *demo_name(int i)
{
    return (0 <= i && i < count) ? demos[i].name : NULL;
}

const char *demo_shot(int i)
{
    return (0 <= i && i < count) ? demos[i].shot : NULL;
}

int demo_coins(int i)
{
    return (0 <= i && i < count) ? demos[i].coins : 0;
}

int demo_state(int i)
{
    return (0 <= i && i < count) ? demos[i].state : 0;
}

int demo_mode(int i)
{
    return (0 <= i && i < count) ? demos[i].mode : 0;
}

int demo_clock(int i)
{
    return (0 <= i && i < count) ? demos[i].timer : 0;
}

time_t demo_date(int i)
{
    return (0 <= i && i < count) ? demos[i].date : 0;
}

const char * demo_str_date(int i)
{
    time_t d = demo_date(i);
    char * res = ctime(&d);
    char * n = strchr(res, '\n');
    if (n) *n = '\0';
    return res;
}

const char * demo_player(int i)
{
    return (0 <= i && i < count) ? demos[i].player : "";
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
		   int mode, int tim, int goal, int score, int balls, int total_time)
{
    int magic = MAGIC;
    int zero  = 0;
    int date  = time(NULL);

    char player[MAXNAM];
    config_get_s(CONFIG_PLAYER, player, MAXNAM);

    /* Initialize the replay file.  Write the header. */

    if (name && (demo_fp = fopen(config_user(name), FMODE_WB)))
    {
        put_index(demo_fp, &magic);
        put_index(demo_fp, &zero);
        put_index(demo_fp, &zero);
        put_index(demo_fp, &zero);
	put_index(demo_fp, &mode);
	put_index(demo_fp, &date);

        fwrite(player, 1, MAXNAM,  demo_fp);
	
        fwrite(shot,   1, PATHMAX, demo_fp);
        fwrite(file,   1, PATHMAX, demo_fp);
        fwrite(back,   1, PATHMAX, demo_fp);
        fwrite(grad,   1, PATHMAX, demo_fp);
        fwrite(song,   1, PATHMAX, demo_fp);

        put_index(demo_fp, &tim);
        put_index(demo_fp, &goal);
        put_index(demo_fp, &score);
        put_index(demo_fp, &balls);
        put_index(demo_fp, &total_time);

        audio_music_fade_to(2.0f, song);

        return game_init(file, back, grad, tim, (goal == 0));
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

void demo_play_stop(int coins, int timer, int state)
{
    if (demo_fp)
    {
        /* Update the demo header using the final coins and time. */

        fseek(demo_fp, 4, SEEK_SET);

        put_index(demo_fp, &timer);
        put_index(demo_fp, &coins);
        put_index(demo_fp, &state);

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
    char src[PATHMAX];
    char dst[PATHMAX];

    if (name && demo_exists(USER_REPLAY_FILE) && strcmp(name, USER_REPLAY_FILE) != 0)
    {
        strncpy(src, config_user(USER_REPLAY_FILE), PATHMAX);
	strncpy(dst, config_user(name),             PATHMAX);

	rename(src, dst);
    }
}

/*---------------------------------------------------------------------------*/

static char demo_replay_name[MAXSTR];

int demo_replay_init(const char *name, int *m, int *s, int *c, int *g, int *t)
{
    struct demo d;

    if ((demo_fp = demo_header(name, &d)))
    {
        strncpy(demo_replay_name, name, MAXSTR);
	if (m) *m = d.mode;
	if (s) *s = d.score;
	if (g) *g = d.goal;
	if (t) *t = d.total_time;

	if (g)
	{
	    audio_music_fade_to(0.5f, d.song);
	    return game_init(d.file, d.back, d.grad, d.time, (d.goal == 0));
	}
	else
	    return game_init(d.file, d.back, d.grad, d.time, 1);
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
