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
#include "set.h"
#include "util.h"
#include "game.h"
#include "level.h"
#include "audio.h"
#include "config.h"

#include "st_set.h"
#include "st_over.h"
#include "st_level.h"
#include "st_start.h"
#include "st_title.h"

/*---------------------------------------------------------------------------*/

#define START_BACK -1
#define START_PRACTICE -2
#define START_NORMAL -3
#define START_CHALLENGE 0

static int shot_id;
static int status_id;

/*---------------------------------------------------------------------------*/

/* Create a level selector button based upon its existence and status. */

static void gui_level(int id, char *text, int i)
{
    int o = level_opened(i);
    int e = level_exists(i);
    int b = level_extra_bonus(i);
    int bo = level_extra_bonus_opened();
    int jd = 0;

    if (!e)
	gui_label(id, text, GUI_SML, GUI_ALL, gui_blk, gui_blk);
    else if (o)
    {
	if (!b)
	    jd = gui_label(id, text, GUI_SML, GUI_ALL, gui_wht, gui_wht);
	else
	    jd = gui_label(id, text, GUI_SML, GUI_ALL, gui_wht, gui_grn);
    }
    else
    {
	if (!b)
	    jd = gui_label(id, text, GUI_SML, GUI_ALL, gui_gry, gui_wht);
	else if (bo)
	    jd = gui_label(id, text, GUI_SML, GUI_ALL, gui_gry, gui_grn);
	else
	    jd = gui_label(id, text, GUI_SML, GUI_ALL, gui_gry, gui_gry);
    }
    
    if (jd)
	gui_active(jd, i, 0);
}

static void start_over_level(i)
{
    int b = level_extra_bonus(i);
    if (i == 0 || level_opened(i))
    {
        gui_set_image(shot_id, level_shot(i));

        set_most_coins(i, -1);
        set_best_times(i, -1);

        if (i == 0)
	    gui_set_label(status_id, _("Challenge all levels from the first one"));
	else if (config_get_d(CONFIG_MODE))
	{
	    if (b)
	        gui_set_label(status_id, _("Play this bonus level in practice mode"));
	    else
	        gui_set_label(status_id, _("Play this level in practice mode"));
	}
	else
	{
	    if (b)
	        gui_set_label(status_id, _("Play this bonus level in normal mode"));
	    else
	        gui_set_label(status_id, _("Play this level in normal mode"));
	}
	return;
    }
    else if (b && !level_extra_bonus_opened())
	gui_set_label(status_id, _("Finish challenge mode to unlock extra bonus levels"));
    else
	gui_set_label(status_id, _("Finish previous levels to unlock this level"));
}

static void start_over(id)
{
    int i;
    if (id == 0)
	return;
    
    i = gui_token(id);
    
    gui_pulse(id, 1.2f);

    switch (i)
    {
    case START_NORMAL:
	gui_set_label(status_id, _("Collect coins and unlock next level"));
	break;
    case START_PRACTICE:
	gui_set_label(status_id, _("Train yourself without time nor coin"));
	break;
    }
    
    if (i >= 0)
        start_over_level(i);
}

/*---------------------------------------------------------------------------*/

static int start_action(int i)
{
    int mode = config_get_d(CONFIG_MODE) ? MODE_PRACTICE : MODE_NORMAL;
    audio_play(AUD_MENU, 1.0f);

    if (i == START_BACK)
        return goto_state(&st_set);
    else if (i == START_NORMAL)
    {
	config_set_d(CONFIG_MODE, 0);
	goto_state(&st_start);
    }
    else if (i == START_PRACTICE)
    {
	config_set_d(CONFIG_MODE, 1);
	goto_state(&st_start);
    }
    
    if (i == START_CHALLENGE)
    {
	i = 1;
	mode = MODE_CHALLENGE;
    }

    if (level_opened(i))
    {
        level_play(i, mode);
        return goto_state(&st_level);
    }
    return 1;
}

static int start_enter(void)
{
    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);
    int m = config_get_d(CONFIG_MODE);

    int id, jd, kd, ld;

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_hstack(id)))
        {
	    
	    gui_label(jd, _(set_name(set_curr())), GUI_SML, GUI_ALL, gui_yel, gui_red);
            gui_filler(jd);
	    if (level_set_completed())
	    {
		gui_label(jd, _("Set Complete"), GUI_SML, GUI_ALL, gui_yel, gui_grn);
                gui_filler(jd);
	    }
            gui_start(jd, _("Back"),  GUI_SML, START_BACK, 0);
        }

	
        if ((jd = gui_harray(id)))
        {
            shot_id = gui_image(jd, level_shot(0), 7 * w / 16, 7 * h / 16);

            if ((kd = gui_varray(jd)))
            {
		gui_state(kd, _("Challenge"), GUI_SML, START_CHALLENGE , 0);
                if ((ld = gui_harray(kd)))
                {
		    gui_state(ld, _("Practice"), GUI_SML, START_PRACTICE, m == 1);
		    gui_state(ld, _("Normal"),   GUI_SML, START_NORMAL,   m == 0);
		}
                if ((ld = gui_harray(kd)))
                {
                    gui_level(ld, "05",  5);
                    gui_level(ld, "04",  4);
                    gui_level(ld, "03",  3);
                    gui_level(ld, "02",  2);
                    gui_level(ld, "01",  1);
                }
                if ((ld = gui_harray(kd)))
                {
                    gui_level(ld, "10", 10);
                    gui_level(ld, "09",  9);
                    gui_level(ld, "08",  8);
                    gui_level(ld, "07",  7);
                    gui_level(ld, "06",  6);
                }
                if ((ld = gui_harray(kd)))
                {
                    gui_level(ld, "15", 15);
                    gui_level(ld, "14", 14);
                    gui_level(ld, "13", 13);
                    gui_level(ld, "12", 12);
                    gui_level(ld, "11", 11);
                }
                if ((ld = gui_harray(kd)))
                {
                    gui_level(ld, "20", 20);
                    gui_level(ld, "19", 19);
                    gui_level(ld, "18", 18);
                    gui_level(ld, "17", 17);
                    gui_level(ld, "16", 16);
                }
                if ((ld = gui_harray(kd)))
                {
                    gui_level(ld, "25", 25);
                    gui_level(ld, "24", 24);
                    gui_level(ld, "23", 23);
                    gui_level(ld, "22", 22);
                    gui_level(ld, "21", 21);
                }
            }
        }
        gui_space(id);


        if ((jd = gui_harray(id)))
        {
            gui_most_coins(jd, 3, 0);
            gui_best_times(jd, 3, 0);
        }
        
	gui_space(id);
	
	status_id = gui_label(id, _("Choose a level to play"), GUI_SML, GUI_ALL, gui_yel, gui_wht);
	
        gui_layout(id, 0, 0);
	
        set_most_coins(0, -1);
        set_best_times(0, -1);

	if (curr_level())
	    start_over_level(curr_level());
    }

    return id;
}

static void start_leave(int id)
{
    gui_delete(id);
}

static void start_paint(int id, float st)
{
    game_draw(0, st);
    config_pop_matrix();
    gui_paint(id);
}

static void start_timer(int id, float dt)
{
    gui_timer(id, dt);
    audio_timer(dt);
}


static void start_point(int id, int x, int y, int dx, int dy)
{
    start_over(gui_point(id, x, y));
}

static void start_stick(int id, int a, int v)
{
    int x = (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a)) ? v : 0;
    int y = (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a)) ? v : 0;
    
    start_over(gui_stick(id, x, y));
}

static int start_click(int b, int d)
{
    if (d && b < 0)
        return start_action(gui_token(gui_click()));
    return 1;
}

static int start_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        return start_action(START_BACK);
    
    if (d && c == SDLK_c)
    {
	level_cheat();
        return goto_state(&st_start);
    }

    if (d && c == SDLK_F12)
    {
        int n = curr_count();
        int i;

        /* Iterate over all levels, taking a screenshot of each. */

        for (i = 1; i < n; i++)
            if (level_exists(i))
                level_snap(i);
    }

    return 1;
}

static int start_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return start_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b) || config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return start_action(START_BACK);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_start = {
    start_enter,
    start_leave,
    start_paint,
    start_timer,
    start_point,
    start_stick,
    start_click,
    start_keybd,
    start_buttn,
    1, 0
};
