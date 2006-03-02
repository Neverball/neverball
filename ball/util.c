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

#include <ctype.h>
#include <string.h>

#include "gui.h"
#include "util.h"
#include "levels.h"
#include "config.h"

/*---------------------------------------------------------------------------*/

static int is_special_name(const char * n)
{
    return (strcmp(n, N_("Hard"))==0 || strcmp(n, N_("Medium"))==0 || strcmp(n, N_("Easy"))==0);
}

/*---------------------------------------------------------------------------*/

static int coin_c[4];
static int coin_n[4];
static int coin_t[4];
static int coin_row;
static int coin_lastrow;

/* Build a Most Coins top three list with default values. */

void gui_most_coins(int id, int n, int n2)
{
    const char *s = "1234567";

    int j, jd, kd, ld, md;

    coin_row = n;
    coin_lastrow = n2;

    if ((jd = gui_hstack(id)))
    {
        gui_filler(jd);

        if ((kd = gui_vstack(jd)))
        {
            gui_label(kd, _("Most Coins"), GUI_SML, GUI_TOP, 0, 0);

            if ((ld = gui_hstack(kd)))
            {
                if ((md = gui_vstack(ld)))
                {
                    for (j = 0; j < n - 1; j++)
                        coin_c[j] = gui_count(md, 1000, GUI_SML, 0);

                    coin_c[j++] = gui_count(md, 1000, GUI_SML, GUI_SE);
		    
		    if (n2)
		    {
                       gui_space(md);
                       coin_c[j++] = gui_count(md, 1000, GUI_SML, GUI_RGT);
		    }
                }

                if ((md = gui_vstack(ld)))
                {
                    for (j = 0; j < n; j++)
			coin_n[j] = gui_label(md, s, GUI_SML, 0, gui_yel, gui_wht);
		    
		    if (n2)
		    {
			gui_space(md);
			coin_n[j++] = gui_label(md, s, GUI_SML, 0, gui_yel, gui_wht);
		    }
                }

                if ((md = gui_vstack(ld)))
                {
                    for (j = 0; j < n - 1; j++)
                        coin_t[j] = gui_clock(md, 359999, GUI_SML, 0);

                    coin_t[j++] = gui_clock(md, 359999,  GUI_SML, GUI_SW);

		    if (n2)
		    {
                       gui_space(md);
                       coin_t[j++] = gui_clock(md, 359999,  GUI_SML, GUI_LFT);
		    }
                }
            }
        }
        gui_filler(jd);
    }
}

/* Set the Most Coins top three list values for level i. */

void set_most_coins(int level, int hilight)
{
    int j, spe;
    const char * name;

    for (j = 0; j < coin_row + coin_lastrow; j++)
    {
	name = level_coin_n(level, j);
	spe = is_special_name(name);

	if (spe)
	   gui_set_color(coin_n[j], 0, 0);
	else if (j != hilight)
	   gui_set_color(coin_n[j], gui_yel, gui_wht);
	else if (j>= coin_row)
	   gui_set_color(coin_n[j], gui_red, gui_red);
	else
	   gui_set_color(coin_n[j], gui_grn, gui_grn);
	
        gui_set_count(coin_c[j], level_coin_c(level, j));
        gui_set_label(coin_n[j], spe ? _(name) : name);
        gui_set_clock(coin_t[j], level_coin_t(level, j));
    }
}

/*---------------------------------------------------------------------------*/

static int time_c[4];
static int time_n[4];
static int time_t[4];
static int time_row;
static int time_lastrow;

/* Build a Best Times top three list with default values. */

void gui_best_times(int id, int n, int n2)
{
    const char *s = "1234567";

    int j, jd, kd, ld, md;

    time_row = n;
    time_lastrow = n2;

    if ((jd = gui_hstack(id)))
    {
        gui_filler(jd);

        if ((kd = gui_vstack(jd)))
        {
            gui_label(kd, _("Best Times"), GUI_SML, GUI_TOP, 0, 0);

            if ((ld = gui_hstack(kd)))
            {
                if ((md = gui_vstack(ld)))
                {
                    for (j = 0; j < n - 1; j++)
                        time_t[j] = gui_clock(md, 359999, GUI_SML, 0);

                    time_t[j++] = gui_clock(md, 359999, GUI_SML, GUI_SE);
		    
		    if (n2)
		    {
                        gui_space(md);
			time_t[j++] = gui_clock(md, 359999, GUI_SML, GUI_RGT);
		    }
                }

                if ((md = gui_vstack(ld)))
                {
                    for (j = 0; j < n; j++)
                        time_n[j] = gui_label(md, s, GUI_SML, 0, gui_yel, gui_wht);

		    if (n2)
		    {
			gui_space(md);
                        time_n[j++] = gui_label(md, s, GUI_SML, 0, gui_yel, gui_wht);
		    }
                }

                if ((md = gui_vstack(ld)))
                {
                    for (j = 0; j < n - 1; j++)
                        time_c[j] = gui_count(md, 1000, GUI_SML, 0);

                    time_c[j++] = gui_count(md, 1000, GUI_SML, GUI_SW);
		    
		    if (n2)
		    {
                        gui_space(md);
			time_c[j++] = gui_count(md, 1000, GUI_SML, GUI_LFT);
		    }
                }
            }
        }
        gui_filler(jd);
    }
}

/* Set the Best Times top three list values for level i. */

void set_best_times(int level, int hilight)
{
    int j, spe;
    const char * name;

    for (j = 0; j < time_row + time_lastrow; j++)
    {
	name = level_time_n(level, j);
	spe = is_special_name(name);
	
	if (spe)
	   gui_set_color(time_n[j], 0, 0);
	else if (j != hilight)
	   gui_set_color(time_n[j], gui_yel, gui_wht);
	else if (j>= time_row)
	   gui_set_color(time_n[j], gui_red, gui_red);
	else
	   gui_set_color(time_n[j], gui_grn, gui_grn);
	
        gui_set_clock(time_t[j], level_time_t(level, j));
        gui_set_label(time_n[j], spe ? _(name) : name);
        gui_set_count(time_c[j], level_time_c(level, j));
    }
}

/*---------------------------------------------------------------------------*/

static int lock = 1;
static int keyd[127]; 

static void gui_fill(int id, char * line)
{
    char l[2];
    l[1] = '\0';
    while(*line)
    {
	*l = *(line++);
	keyd[(int)*l] = gui_state(id, l, GUI_SML, *l, 0);
    }
}

void gui_keyboard(int id)
{
    int jd, kd, ld;

    lock = 1;

    if ((jd = gui_hstack(id)))
    {
        gui_filler(jd);

        if ((kd = gui_vstack(jd)))
        {
            if ((ld = gui_harray(kd)))
            {
                gui_filler(ld);
		gui_fill(ld, "9876543210");
                gui_filler(ld);
            }
            if ((ld = gui_harray(kd)))
            {
                gui_filler(ld);
		gui_fill(ld, "JIHGFEDCBA");
                gui_filler(ld);
            }
            if ((ld = gui_harray(kd)))
            {
                gui_filler(ld);
		gui_fill(ld, "TSRQPONMLK");
                gui_filler(ld);
            }
            if ((ld = gui_harray(kd)))
            {
                gui_filler(ld);
		gui_fill(ld, "!.-_ZYXWVU");
                gui_filler(ld);
            }
            if ((ld = gui_harray(kd)))
            {
                gui_filler(ld);
		gui_fill(ld, "jihgfedcba");
                gui_filler(ld);
            }
            if ((ld = gui_harray(kd)))
            {
                gui_filler(ld);
		gui_fill(ld, "tsrqponmlk");
                gui_filler(ld);
            }
            if ((ld = gui_hstack(kd)))
            {
                gui_filler(ld);
                gui_state(ld, _("del"), GUI_SML, GUI_BS, 0);
		gui_fill(ld, "zyxwvu");
                gui_filler(ld);
            }
        }
        gui_filler(jd);
    }
}

/*---------------------------------------------------------------------------*/
