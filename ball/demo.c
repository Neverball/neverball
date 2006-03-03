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

#include "demo.h"
#include "game.h"
#include "audio.h"
#include "solid.h"
#include "config.h"
#include "binary.h"

/*---------------------------------------------------------------------------*/

#define MAGIC     0x4E425251 /* Replay file magic number (should not change) */
#define OLD_MAGIC 0x4E425250 /* Replay file magic number for neverball 1.4.0 */
#define REPLAY_VERSION  1    /* Replay file format version (can change)      */

#define DEMO_FPS_CAP 200 /* FPS replay limit, keeps size down on monster systems */

static FILE *demo_fp;


/* Demo information structure (header) */
struct demo
{
    char   name[MAXNAM];      /* demo basename */
    char   filename[PATHMAX]; /* demo path */

    /* The following reflects the file structure */
    /* magic number */
    /* replay file version */
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
    int    times;           /* total time (challenge mode) */
    char   nb_version[20]; /* neverball version used */
};


static struct demo demos[MAXDEMO]; /* Array of scanned demos */

static int count; /* number of scanned demos */

/*---------------------------------------------------------------------------*/

void demo_dump_info(const struct demo * d)
/* This function dump the info of a demo structure*/
{
    printf("Name:         %s\n"
	   "File:         %s\n"
	   "NB Version:   %s\n"
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
	   d->nb_version,
	   d->timer, d->coins, d->mode, d->state, ctime(&d->date),
	   d->player,
	   d->shot, d->file, d->back, d->grad, d->song,
	   d->time, d->goal, d->score, d->balls, d->times);
}

FILE * demo_header_read(const char * filename, struct demo * d)
/* Open a demo file, fill the demo information structure
 * If success, return the file pointer positioned after the header
 * If fail, return null 
 */
{
    FILE *fp;
    char * basename;
    if ((fp = fopen(filename, FMODE_RB)))
    {
	int magic;
	int version;
	int t;

	get_index(fp, &magic);
	get_index(fp, &version);
	get_index(fp, &t);  /* if time is 0, it's mean the replay wat not finished */

	if (magic == MAGIC && version == REPLAY_VERSION && t)
	{
#ifdef _WIN32
	    basename = strrchr(filename, '\\');
#else
	    basename = strrchr(filename, '/');
#endif
	    if (basename != NULL)
	        strncpy(d->name, basename+1, MAXNAM);
	    else
	        strncpy(d->name, filename, MAXNAM);
			
	    strncpy(d->filename, filename, PATHMAX);
	    
	    d->timer = t;
	    get_index (fp, &d->coins);
	    get_index (fp, &d->state);
	    get_index (fp, &d->mode);
	    get_index (fp, (int*)&d->date);
	    get_string(fp, d->player, MAXNAM);
	    get_string(fp, d->shot,   PATHMAX);
	    get_string(fp, d->file,   PATHMAX);
	    get_string(fp, d->back,   PATHMAX);
	    get_string(fp, d->grad,   PATHMAX);
	    get_string(fp, d->song,   PATHMAX);
            get_index (fp, &d->time);
            get_index (fp, &d->goal);
            get_index (fp, &d->score);
            get_index (fp, &d->balls);
            get_index (fp, &d->times);
	    get_string(fp, d->nb_version, 20);

	    return fp;
	}
	fclose(fp);
    }
    return NULL;
}

static FILE * demo_header_write(struct demo * d)
/* Create a new demo file, write the demo information structure
 * If success, return the file pointer positioned after the header
 * If fail, return null
 * */
{
    int magic = MAGIC;
    int version = REPLAY_VERSION;
    int zero  = 0;
    FILE *fp;
    
    if (d->filename && (fp = fopen(d->filename, FMODE_WB)))
    {
        put_index (fp, &magic);
        put_index (fp, &version);
        put_index (fp, &zero);
        put_index (fp, &zero);
        put_index (fp, &zero);
	put_index (fp, &d->mode);
	put_index (fp, (int*)&d->date);
	put_string(fp, d->player);
	put_string(fp, d->shot);
	put_string(fp, d->file);
	put_string(fp, d->back);
	put_string(fp, d->grad);
	put_string(fp, d->song);
        put_index (fp, &d->time);
        put_index (fp, &d->goal);
        put_index (fp, &d->score);
        put_index (fp, &d->balls);
        put_index (fp, &d->times);
	put_string(fp, VERSION);

        return fp;
    }
    return NULL;
}

void demo_header_stop(FILE * fp, int coins, int timer, int state)
    /* Update the demo header using the final level state. */
{
    long pos = ftell(fp);
    fseek(fp, 8, SEEK_SET);
    put_index(fp, &timer);
    put_index(fp, &coins);
    put_index(fp, &state);
    fseek(fp, pos, SEEK_SET);
}
	
/*---------------------------------------------------------------------------*/

static void demo_scan_file(const char * filename)
/* Scan a other file (used by demo_scan */
{
    FILE *fp;
    if ((fp = demo_header_read(config_user(filename), &demos[count])))
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

    return (n > 0) ? demos[(rand() >> 4) % n].filename : NULL;
}

const char *demo_filename(int i)
{
    return (0 <= i && i < count) ? demos[i].filename : NULL;
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

void demo_str_date(int i, char * str, int len)
{
    time_t d = demo_date(i);
    struct tm * tm = localtime(&d);
    strftime (str, len, "%c",tm);
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
		   const struct level * level,
		   const struct level_game * lg)
{
    struct demo demo;

    /* file structure */
    strncpy(demo.name, name, MAXNAM);
    strncpy(demo.filename, config_user(name), PATHMAX);
    demo.time = demo.coins = demo.state = 0;
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

    demo_fp = demo_header_write(&demo);
    if (demo_fp == NULL)
	return 0;
    else
    {
        audio_music_fade_to(2.0f, level->song);
        return game_init(level, lg->time, lg->goal);
    }
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

void demo_play_stop(const struct level_game *lg)
/* Update the demo header using the final level state. */
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

static int demo_load_level(const struct demo * demo, struct level * level)
/* Load the level of the demo and fill the level structure */
{
    strcpy(level->file, demo->file);
    strcpy(level->back, demo->back);
    strcpy(level->grad, demo->grad);
    strcpy(level->shot, demo->shot);
    strcpy(level->song, demo->song);
    level->time = demo->time;
    level->goal = demo->goal;
    return 1;
}

static struct demo  demo_replay;       /* The current demo */
static struct level demo_level_replay; /* The current level demo-ed*/

int demo_replay_init(const char *name, struct level_game *lg)
/* Internally load a replay an fill the lg structure (if not NULL) */
{
    if ((demo_fp = demo_header_read(name, &demo_replay)))
    {
	demo_load_level(&demo_replay, &demo_level_replay);
	
	if (lg)
	{
	    lg->mode       = demo_replay.mode;
	    lg->score      = demo_replay.score;
	    lg->times      = demo_replay.times;
	    lg->time       = demo_replay.time;
	    lg->goal       = demo_replay.goal;
	    
	    /* A normal replay demo */
	    audio_music_fade_to(0.5f, demo_replay.song);
	    return game_init(&demo_level_replay, demo_replay.time, demo_replay.goal);
	}
	else /* A title screen demo */
	    return game_init(&demo_level_replay, demo_replay.time, 0);
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

        if (d) unlink(demo_replay.filename);
    }
}

void demo_replay_dump_info(void)
{
    demo_dump_info(&demo_replay);
}

/*---------------------------------------------------------------------------*/
