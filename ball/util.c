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

#include "gui.h"
#include "level.h"

/*---------------------------------------------------------------------------*/
/*
void gui_curr_score(int id, int i)
{
    int jd;

    if ((jd = gui_harray(id)))
    {
        gui_count(jd, level_coin_c(i, -1), GUI_MED, GUI_NE | GUI_SE);
        gui_clock(jd, level_time_t(i, -1), GUI_MED, GUI_NW | GUI_SW);
    }
}
*/
/*---------------------------------------------------------------------------*/

static int coin_c[4];
static int coin_n[4];
static int coin_t[4];

/* Build a Most Coins top three list with default values. */

void gui_most_coins(int id, int n, int i)
{
    const char *s = "1234567";

    const float *c0 = gui_yel;
    const float *c1 = gui_grn;
    const float *c2 = gui_wht;

    int j, jd, kd, ld, md;

    if ((jd = gui_hstack(id)))
    {
        gui_filler(jd);

        if ((kd = gui_vstack(jd)))
        {
            gui_label(kd, "Most Coins", GUI_SML, GUI_TOP, 0, 0);

            if ((ld = gui_hstack(kd)))
            {
                if ((md = gui_varray(ld)))
                {
                    for (j = 0; j < n - 1; j++)
                        coin_c[j] = gui_count(md, 1000, GUI_SML, 0);

                    coin_c[j] = gui_count(md, 1000, GUI_SML, GUI_SE);
                }

                if ((md = gui_varray(ld)))
                {
                    for (j = 0; j < n; j++)
                        if (j == i)
                            coin_n[j] = gui_label(md, s, GUI_SML, 0, c1, c1);
                        else
                            coin_n[j] = gui_label(md, s, GUI_SML, 0, c0, c2);
                }

                if ((md = gui_varray(ld)))
                {
                    for (j = 0; j < n - 1; j++)
                        coin_t[j] = gui_clock(md, 359999, GUI_SML, 0);

                    coin_t[j] = gui_clock(md, 359999,  GUI_SML, GUI_SW);
                }
            }
        }
        gui_filler(jd);
    }
}

/* Set the Most Coins top three list values for level i. */

void set_most_coins(int level, int n)
{
    int j;

    for (j = 0; j < n; j++)
    {
        gui_set_count(coin_c[j], level_coin_c(level, j));
        gui_set_label(coin_n[j], level_coin_n(level, j));
        gui_set_clock(coin_t[j], level_coin_t(level, j));
    }
}

/*---------------------------------------------------------------------------*/

static int time_c[4];
static int time_n[4];
static int time_t[4];

/* Build a Best Times top three list with default values. */

void gui_best_times(int id, int n, int i)
{
    const char *s = "1234567";

    const float *c0 = gui_yel;
    const float *c1 = gui_grn;
    const float *c2 = gui_wht;

    int j, jd, kd, ld, md;

    if ((jd = gui_hstack(id)))
    {
        gui_filler(jd);

        if ((kd = gui_vstack(jd)))
        {
            gui_label(kd, "Best Times", GUI_SML, GUI_TOP, 0, 0);

            if ((ld = gui_hstack(kd)))
            {
                if ((md = gui_varray(ld)))
                {
                    for (j = 0; j < n - 1; j++)
                        time_t[j] = gui_clock(md, 359999, GUI_SML, 0);

                    time_t[j] = gui_clock(md, 359999, GUI_SML, GUI_SE);
                }

                if ((md = gui_varray(ld)))
                {
                    for (j = 0; j < n; j++)
                        if (i == j)
                            time_n[j] = gui_label(md, s, GUI_SML, 0, c1, c1);
                        else
                            time_n[j] = gui_label(md, s, GUI_SML, 0, c0, c2);
                }

                if ((md = gui_varray(ld)))
                {
                    for (j = 0; j < n - 1; j++)
                        time_c[j] = gui_count(md, 1000, GUI_SML, 0);

                    time_c[j] = gui_count(md, 1000, GUI_SML, GUI_SW);
                }
            }
        }
        gui_filler(jd);
    }
}

/* Set the Best Times top three list values for level i. */

void set_best_times(int level, int n)
{
    int j;

    for (j = 0; j < n; j++)
    {
        gui_set_clock(time_t[j], level_time_t(level, j));
        gui_set_label(time_n[j], level_time_n(level, j));
        gui_set_count(time_c[j], level_time_c(level, j));
    }
}

/*---------------------------------------------------------------------------*/
