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

#define MAXSTR 256
#define MAXLVL 32

struct level
{
    char file[MAXSTR];
    char back[MAXSTR];
    int  time;
};

static struct level level_v[MAXLVL];
static int          level_i = 0;
static int          level_n = 0;

/*---------------------------------------------------------------------------*/

void level_init(void)
{
    if (level_n == 0)
    {
        FILE *fin = fopen("data/levels.txt", "r");

        if (fin)
        {
            while (fscanf(fin, "%s %s %d",
                           level_v[level_n].file,
                           level_v[level_n].back,
                          &level_v[level_n].time) == 3)
                level_n++;

            fclose(fin);
        }
    }

    level_i = 0;

    back_init(level_v[level_i].back);
    game_load(level_v[level_i].file,
              level_v[level_i].time);
}

int level_pass(void)
{
    game_free();
    back_free();

    if (++level_i < level_n)
    {
        back_init(level_v[level_i].back);
        game_load(level_v[level_i].file,
                  level_v[level_i].time);

        return 1;
    }
    return 0;
}

int level_fail(void)
{
    game_free();
    back_free();

    if (game_fail())
    {
        back_init(level_v[level_i].back);
        game_load(level_v[level_i].file,
                  level_v[level_i].time);

        return 1;
    }
    return 0;
}
