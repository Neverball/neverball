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

#include "set.h"
#include "game.h"
#include "level.h"
#include "solid.h"
#include "config.h"

static FILE *demo_fp;

/*---------------------------------------------------------------------------*/

int demo_exists(void)
{
    char filename[MAXSTR];

    /* Confirm the existence of a readable replay file. */

    if (config_home(filename, USER_REPLAY_FILE, MAXSTR))
        if ((demo_fp = fopen(filename, FMODE_RB)))
        {
            fclose(demo_fp);
            demo_fp = NULL;
            return 1;
        }

    return 0;
}

/*---------------------------------------------------------------------------*/

void demo_play_init(void)
{
    char filename[MAXSTR];

    /* Initialize the replay file.  Write the header. */

    if (config_home(filename, USER_REPLAY_FILE, MAXSTR))
        if ((demo_fp = fopen(filename, FMODE_WB)))
        {
            fputc(set_curr(),   demo_fp);
            fputc(curr_score(), demo_fp);
            fputc(curr_coins(), demo_fp);
            fputc(curr_balls(), demo_fp);
            fputc(curr_level(), demo_fp);
        }
}

void demo_play_step(float dt)
{
    if (demo_fp)
    {
        float_put(demo_fp, &dt);
        game_put(demo_fp);
    }
}

/*---------------------------------------------------------------------------*/

void demo_replay_init(void)
{
    char filename[MAXSTR];

    if (config_home(filename, USER_REPLAY_FILE, MAXSTR))
        if ((demo_fp = fopen(filename, FMODE_RB)))
        {
            int l, b, c, s, n = fgetc(demo_fp);

            if (set_exists(n))
            {
                set_goto(n);

                s = fgetc(demo_fp);
                c = fgetc(demo_fp);
                b = fgetc(demo_fp);
                l = fgetc(demo_fp);

                if (level_exists(l))
                {
                    level_goto(s, c, b, l);
                    level_song();
                }
            }
        }
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

/*---------------------------------------------------------------------------*/

void demo_finish(void)
{
    if (demo_fp)
    {
        fclose(demo_fp);
        demo_fp = NULL;
    }
}

/*---------------------------------------------------------------------------*/
