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

void gui_curr_score(int id, int i)
{
    int jd;

    if ((jd = gui_harray(id)))
    {
        gui_count(jd, level_coin_c(i, -1), GUI_MED, GUI_NE | GUI_SE);
        gui_clock(jd, level_time_t(i, -1), GUI_MED, GUI_NW | GUI_SW);
    }
}

/*---------------------------------------------------------------------------*/

static int coin_1c;
static int coin_2c;
static int coin_3c;

static int coin_1n;
static int coin_2n;
static int coin_3n;

static int coin_1t;
static int coin_2t;
static int coin_3t;

/* Build a Most Coins top three list with default values. */

void gui_most_coins(int id)
{
    const float *c0 = gui_yel;
    const float *c1 = gui_wht;

    int jd, kd, ld, md;

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
                    coin_1c = gui_count(md, 1000,    GUI_SML, 0);
                    coin_2c = gui_count(md, 1000,    GUI_SML, 0);
                    coin_3c = gui_count(md, 1000,    GUI_SML, GUI_SE);
                }
                if ((md = gui_varray(ld)))
                {
                    coin_1n = gui_label(md, "Hard",  GUI_SML, 0, c0, c1);
                    coin_2n = gui_label(md, "Medium",GUI_SML, 0, c0, c1);
                    coin_3n = gui_label(md, "Easy",  GUI_SML, 0, c0, c1);
                }
                if ((md = gui_varray(ld)))
                {
                    coin_1t = gui_clock(md, 359999,  GUI_SML, 0);
                    coin_2t = gui_clock(md, 359999,  GUI_SML, 0);
                    coin_3t = gui_clock(md, 359999,  GUI_SML, GUI_SW);
                }
            }
        }
        gui_filler(jd);
    }
}

/* Set the Most Coins top three list values for level i. */

void set_most_coins(int i)
{
    gui_set_count(coin_1c, level_coin_c(i, 0));
    gui_set_count(coin_2c, level_coin_c(i, 1));
    gui_set_count(coin_3c, level_coin_c(i, 2));

    gui_set_label(coin_1n, level_coin_n(i, 0));
    gui_set_label(coin_2n, level_coin_n(i, 1));
    gui_set_label(coin_3n, level_coin_n(i, 2));

    gui_set_clock(coin_1t, level_coin_t(i, 0));
    gui_set_clock(coin_2t, level_coin_t(i, 1));
    gui_set_clock(coin_3t, level_coin_t(i, 2));
}

/*---------------------------------------------------------------------------*/

static int time_1c;
static int time_2c;
static int time_3c;

static int time_1n;
static int time_2n;
static int time_3n;

static int time_1t;
static int time_2t;
static int time_3t;

/* Build a Best Times top three list with default values. */

void gui_best_times(int id)
{
    const float *c0 = gui_yel;
    const float *c1 = gui_wht;

    int jd, kd, ld, md;

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
                    time_1t = gui_clock(md, 359999,  GUI_SML, 0);
                    time_2t = gui_clock(md, 359999,  GUI_SML, 0);
                    time_3t = gui_clock(md, 359999,  GUI_SML, GUI_SE);
                }
                if ((md = gui_varray(ld)))
                {
                    time_1n = gui_label(md, "Hard",  GUI_SML, 0, c0, c1);
                    time_2n = gui_label(md, "Medium",GUI_SML, 0, c0, c1);
                    time_3n = gui_label(md, "Easy",  GUI_SML, 0, c0, c1);
                }
                if ((md = gui_varray(ld)))
                {
                    time_1c = gui_count(md, 1000,    GUI_SML, 0);
                    time_2c = gui_count(md, 1000,    GUI_SML, 0);
                    time_3c = gui_count(md, 1000,    GUI_SML, GUI_SW);
                }
            }
        }
        gui_filler(jd);
    }
}

/* Set the Best Times top three list values for level i. */

void set_best_times(int i)
{
    gui_set_clock(time_1t, level_time_t(i, 0));
    gui_set_clock(time_2t, level_time_t(i, 1));
    gui_set_clock(time_3t, level_time_t(i, 2));

    gui_set_label(time_1n, level_time_n(i, 0));
    gui_set_label(time_2n, level_time_n(i, 1));
    gui_set_label(time_3n, level_time_n(i, 2));

    gui_set_count(time_1c, level_time_c(i, 0));
    gui_set_count(time_2c, level_time_c(i, 1));
    gui_set_count(time_3c, level_time_c(i, 2));
}

/*---------------------------------------------------------------------------*/
