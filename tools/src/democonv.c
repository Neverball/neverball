/*
 * democonv -- Neverball 1.4.0 to v3 format replay converter.
 *
 * Copyright (C) 2006 Jānis Rūcis
 *
 * Part of the Neverball project.
 *
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

/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include <SDL_byteorder.h>

#include "base_config.h"
#include "binary.h"
#include "level.h"

/*----------------------------------------------------------------------------*/

#define DATELEN 20

void get_short(FILE *fin, short *s)
{
    unsigned char *p = (unsigned char *) s;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    p[1] = (unsigned char) fgetc(fin);
    p[0] = (unsigned char) fgetc(fin);
#else
    p[0] = (unsigned char) fgetc(fin);
    p[1] = (unsigned char) fgetc(fin);
#endif
}

/*---------------------------------------------------------------------------*/

#define MAG_OLD     0x4E425250

#define MAG_NEW     0x52424EAF
#define VER_NEW     3

/*---------------------------------------------------------------------------*/

int read_demo(FILE *fin, int *timer, int *coins,
              int *time, int *goal, int *score, int *balls,
              char *file, char *shot)
{
    short t, c;
    short zero;

    short st, sg, ss, sb;

    char none[PATHMAX];

    get_short(fin, &t);
    get_short(fin, &c);

    fread(shot, 1, PATHMAX, fin);
    fread(file, 1, PATHMAX, fin);
    fread(none, 1, PATHMAX, fin);
    fread(none, 1, PATHMAX, fin);
    fread(none, 1, PATHMAX, fin);

    get_short(fin, &st);
    get_short(fin, &sg);
    get_short(fin, &ss);
    get_short(fin, &zero);
    get_short(fin, &sb);

    if (!feof(fin))
    {
        *timer = (int) t;
        *coins = (int) c;

        *time  = (int) st;
        *goal  = (int) sg;
        *score = (int) ss;
        *balls = (int) sb;
        
        return 1;
    }
    return 0;
}

void parse_args(int argc, char *argv[],
                int  *state,
                int  *mode,
                char *player,
                char *date)
{
    int i;

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i],      "--goal") == 0)
            *state = GAME_GOAL;
        else if (strcmp(argv[i], "--time-out") == 0)
            *state = GAME_TIME;
        else if (strcmp(argv[i], "--fall-out") == 0)
            *state = GAME_FALL;

        else if (strcmp(argv[i], "--best-time") == 0)
            *mode  = MODE_PRACTICE;
        else if (strcmp(argv[i], "--most-coins") == 0)
            *mode  = MODE_NORMAL;
        else if (strcmp(argv[i], "--freestyle") == 0)
            *mode  = 0;

        else if (strcmp(argv[i], "--player") == 0)
            strncpy(player, argv[++i], MAXNAM);
        else if (strcmp(argv[i], "--date") == 0)
            strncpy(date, argv[++i], DATELEN);
    }
    return;
}

/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    FILE *fin, *fout;

    int magic;
    int version = VER_NEW;

    int  state          = GAME_NONE;
    int  mode           = 0;
    char player[MAXNAM] = "Player";
    char date[DATELEN]  = "2003-07-16T00:00:00";

    parse_args(argc, argv, &state, &mode, player, date);

    fin  = stdin;
    fout = stdout;

    get_index(fin, &magic);

    if (magic == MAG_OLD)
    {
        int timer;
        int coins;

        int time, goal, score, balls;

        char file[PATHMAX];
        char shot[PATHMAX];

        int zero = 0;

        if (read_demo(fin, &timer, &coins,
                      &time, &goal, &score, &balls,
                      file, shot))
        {
            magic = MAG_NEW;

            put_index(fout, &magic);
            put_index(fout, &version);

            put_index(fout, &timer);
            put_index(fout, &coins);
            put_index(fout, &state);
            put_index(fout, &mode);

            fwrite(player, 1, MAXNAM,  fout);
            fwrite(date,   1, DATELEN, fout);

            fwrite(shot, 1, PATHMAX, fout);
            fwrite(file, 1, PATHMAX, fout);

            put_index(fout, &time);
            put_index(fout, &goal);
            put_index(fout, &score);
            put_index(fout, &balls);
            put_index(fout, &zero);

            while (!feof(fin))
                fputc(fgetc(fin), fout);

            return 0;
        }
    }
    return 1;
}

/*---------------------------------------------------------------------------*/
