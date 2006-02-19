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
#include "level.h"
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

void gui_keyboard(int id)
{
    int jd, kd, ld;

    lock = 1;

    if ((jd = gui_hstack(id)))
    {
        gui_filler(jd);

        if ((kd = gui_vstack(jd)))
        {
            if ((ld = gui_hstack(kd)))
            {
                gui_filler(ld);

                keyd['9'] = gui_state(ld, "9", GUI_SML, '9', 0);
                keyd['8'] = gui_state(ld, "8", GUI_SML, '8', 0);
                keyd['7'] = gui_state(ld, "7", GUI_SML, '7', 0);
                keyd['6'] = gui_state(ld, "6", GUI_SML, '6', 0);
                keyd['5'] = gui_state(ld, "5", GUI_SML, '5', 0);
                keyd['4'] = gui_state(ld, "4", GUI_SML, '4', 0);
                keyd['3'] = gui_state(ld, "3", GUI_SML, '3', 0);
                keyd['3'] = gui_state(ld, "2", GUI_SML, '2', 0);
                keyd['1'] = gui_state(ld, "1", GUI_SML, '1', 0);
                keyd['0'] = gui_state(ld, "0", GUI_SML, '0', 0);
                gui_filler(ld);
            }
            if ((ld = gui_hstack(kd)))
            {
                gui_filler(ld);
                keyd['J'] = gui_state(ld, "J", GUI_SML, 'J', 0);
                keyd['I'] = gui_state(ld, "I", GUI_SML, 'I', 0);
                keyd['H'] = gui_state(ld, "H", GUI_SML, 'H', 0);
                keyd['G'] = gui_state(ld, "G", GUI_SML, 'G', 0);
                keyd['F'] = gui_state(ld, "F", GUI_SML, 'F', 0);
                keyd['E'] = gui_state(ld, "E", GUI_SML, 'E', 0);
                keyd['D'] = gui_state(ld, "D", GUI_SML, 'D', 0);
                keyd['C'] = gui_state(ld, "C", GUI_SML, 'C', 0);
                keyd['B'] = gui_state(ld, "B", GUI_SML, 'B', 0);
                keyd['A'] = gui_state(ld, "A", GUI_SML, 'A', 0);
                gui_filler(ld);
            }
            if ((ld = gui_hstack(kd)))
            {
                gui_filler(ld);
                keyd['T'] = gui_state(ld, "T", GUI_SML, 'T', 0);
                keyd['S'] = gui_state(ld, "S", GUI_SML, 'S', 0);
                keyd['R'] = gui_state(ld, "R", GUI_SML, 'R', 0);
                keyd['Q'] = gui_state(ld, "Q", GUI_SML, 'Q', 0);
                keyd['P'] = gui_state(ld, "P", GUI_SML, 'P', 0);
                keyd['O'] = gui_state(ld, "O", GUI_SML, 'O', 0);
                keyd['N'] = gui_state(ld, "N", GUI_SML, 'N', 0);
                keyd['M'] = gui_state(ld, "M", GUI_SML, 'M', 0);
                keyd['L'] = gui_state(ld, "L", GUI_SML, 'L', 0);
                keyd['K'] = gui_state(ld, "K", GUI_SML, 'K', 0);
                gui_filler(ld);
            }
            if ((ld = gui_hstack(kd)))
            {
                gui_filler(ld);
                gui_state(ld, "<", GUI_SML, GUI_BS, 0);
                keyd['Z'] = gui_state(ld, "Z", GUI_SML, 'Z', 0);
                keyd['Y'] = gui_state(ld, "Y", GUI_SML, 'Y', 0);
                keyd['X'] = gui_state(ld, "X", GUI_SML, 'X', 0);
                keyd['W'] = gui_state(ld, "W", GUI_SML, 'W', 0);
                keyd['V'] = gui_state(ld, "V", GUI_SML, 'V', 0);
                keyd['U'] = gui_state(ld, "U", GUI_SML, 'U', 0);
                gui_state(ld, _("caps"), GUI_SML, GUI_CL, 0);
                gui_filler(ld);
            }
        }
        gui_filler(jd);
    }
}

void gui_keyboard_lock(void)
{
    lock = lock ? 0 : 1;

    gui_set_label(keyd['A'], lock ? "A" : "a");
    gui_set_label(keyd['B'], lock ? "B" : "b");
    gui_set_label(keyd['C'], lock ? "C" : "c");
    gui_set_label(keyd['D'], lock ? "D" : "d");
    gui_set_label(keyd['E'], lock ? "E" : "e");
    gui_set_label(keyd['F'], lock ? "F" : "f");
    gui_set_label(keyd['G'], lock ? "G" : "g");
    gui_set_label(keyd['H'], lock ? "H" : "h");
    gui_set_label(keyd['I'], lock ? "I" : "i");
    gui_set_label(keyd['J'], lock ? "J" : "j");
    gui_set_label(keyd['K'], lock ? "K" : "k");
    gui_set_label(keyd['L'], lock ? "L" : "l");
    gui_set_label(keyd['M'], lock ? "M" : "m");
    gui_set_label(keyd['N'], lock ? "N" : "n");
    gui_set_label(keyd['O'], lock ? "O" : "o");
    gui_set_label(keyd['P'], lock ? "P" : "p");
    gui_set_label(keyd['Q'], lock ? "Q" : "q");
    gui_set_label(keyd['R'], lock ? "R" : "r");
    gui_set_label(keyd['S'], lock ? "S" : "s");
    gui_set_label(keyd['T'], lock ? "T" : "t");
    gui_set_label(keyd['U'], lock ? "U" : "u");
    gui_set_label(keyd['V'], lock ? "V" : "v");
    gui_set_label(keyd['W'], lock ? "W" : "w");
    gui_set_label(keyd['X'], lock ? "X" : "x");
    gui_set_label(keyd['Y'], lock ? "Y" : "y");
    gui_set_label(keyd['Z'], lock ? "Z" : "z");
}

char gui_keyboard_char(char c)
{
    return lock ? toupper(c) : tolower(c);
}

/*---------------------------------------------------------------------------*/
