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
#include "util.h"
#include "level.h"

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

void gui_keyboard(int id)
{
    int jd, kd, ld;

    if ((jd = gui_hstack(id)))
    {
        gui_filler(jd);

        if ((kd = gui_vstack(jd)))
        {
            if ((ld = gui_hstack(kd)))
            {
                gui_state(ld, "9", GUI_SML, '9', 0);
                gui_state(ld, "8", GUI_SML, '8', 0);
                gui_state(ld, "7", GUI_SML, '7', 0);
                gui_state(ld, "6", GUI_SML, '6', 0);
                gui_state(ld, "5", GUI_SML, '5', 0);
                gui_state(ld, "4", GUI_SML, '4', 0);
                gui_state(ld, "3", GUI_SML, '3', 0);
                gui_state(ld, "2", GUI_SML, '2', 0);
                gui_state(ld, "1", GUI_SML, '1', 0);
                gui_state(ld, "0", GUI_SML, '0', 0);
            }
            if ((ld = gui_hstack(kd)))
            {
                gui_state(ld, "J", GUI_SML, 'J', 0);
                gui_state(ld, "I", GUI_SML, 'I', 0);
                gui_state(ld, "H", GUI_SML, 'H', 0);
                gui_state(ld, "G", GUI_SML, 'G', 0);
                gui_state(ld, "F", GUI_SML, 'F', 0);
                gui_state(ld, "E", GUI_SML, 'E', 0);
                gui_state(ld, "D", GUI_SML, 'D', 0);
                gui_state(ld, "C", GUI_SML, 'C', 0);
                gui_state(ld, "B", GUI_SML, 'B', 0);
                gui_state(ld, "A", GUI_SML, 'A', 0);
            }
            if ((ld = gui_hstack(kd)))
            {
                gui_state(ld, "T", GUI_SML, 'T', 0);
                gui_state(ld, "S", GUI_SML, 'S', 0);
                gui_state(ld, "R", GUI_SML, 'R', 0);
                gui_state(ld, "Q", GUI_SML, 'Q', 0);
                gui_state(ld, "P", GUI_SML, 'P', 0);
                gui_state(ld, "O", GUI_SML, 'O', 0);
                gui_state(ld, "N", GUI_SML, 'N', 0);
                gui_state(ld, "M", GUI_SML, 'M', 0);
                gui_state(ld, "L", GUI_SML, 'L', 0);
                gui_state(ld, "K", GUI_SML, 'K', 0);
            }
            if ((ld = gui_hstack(kd)))
            {
                gui_filler(ld);
                gui_state(ld, "<", GUI_SML, GUI_BS, 0);
                gui_state(ld, "Z", GUI_SML, 'Z', 0);
                gui_state(ld, "Y", GUI_SML, 'Y', 0);
                gui_state(ld, "X", GUI_SML, 'X', 0);
                gui_state(ld, "W", GUI_SML, 'W', 0);
                gui_state(ld, "V", GUI_SML, 'V', 0);
                gui_state(ld, "U", GUI_SML, 'U', 0);
                gui_filler(ld);
            }
        }

        gui_filler(jd);
    }
}

/*---------------------------------------------------------------------------*/
