/*   
 *   Copyright (C) 2003 Robert Kooima
 *
 *   SUPER EMPTY BALL is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by the
 *   Free Software Foundation;  either version 2 of the  License, or (at your
 *   option) any later version.
 *
 *   This program  is distributed  in the  hope that it  will be  useful, but
 *   WITHOUT   ANY   WARRANTY;  without   even   the   implied  warranty   of
 *   MERCHANTABILITY  or  FITNESS FOR  A  PARTICULAR  PURPOSE.   See the  GNU
 *   General Public License for more details.
 */

#include <stdio.h>

#include "game.h"
#include "geom.h"

/*---------------------------------------------------------------------------*/

int score;
int balls;
int level;
int count;

/*---------------------------------------------------------------------------*/

#define MAXSTR 256
#define MAXLVL 32

struct level
{
    char file[MAXSTR];
    char back[MAXSTR];
    int  time;
};

static struct level level_v[MAXLVL];

/*---------------------------------------------------------------------------*/

void level_init(void)
{
    FILE *fin = fopen("data/levels.txt", "r");

    count = 0;
    level = 0;
    score = 0;
    balls = 2;

    if (fin)
    {
        while (fscanf(fin, "%s %s %d",
                       level_v[count].file,
                       level_v[count].back,
                      &level_v[count].time) == 3)
            count++;

        fclose(fin);
    }
}

void level_goto(int i)
{
    level = i;

    back_init(level_v[level].back);
    game_init(level_v[level].file,
              level_v[level].time);
}

void level_free(void)
{
    game_free();
    back_free();

    count = 0;
}

int level_pass(void)
{
    if (++level < count)
    {
        game_free();
        back_free();

        back_init(level_v[level].back);
        game_init(level_v[level].file,
                  level_v[level].time);

        return 1;
    }
    return 0;
}

int level_fail(void)
{
    if (--balls >= 0)
    {
        game_free();
        back_free();

        back_init(level_v[level].back);
        game_init(level_v[level].file,
                  level_v[level].time);

        return 1;
    }
    return 0;
}
