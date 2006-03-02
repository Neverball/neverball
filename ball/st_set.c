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
#include "game.h"
#include "audio.h"
#include "config.h"
#include "st_shared.h"

#include "st_set.h"
#include "st_title.h"
#include "st_start.h"

/*---------------------------------------------------------------------------*/

#define SET_BACK -1
#define SET_PREV -2
#define SET_NEXT -3

#define SET_GROUP 5 /* number of sets in one screen */

static int shot_id;
static int desc_id;

static int set_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch(i)
    {
    case SET_BACK:
        return goto_state(&st_title);

    case SET_PREV:
	config_set_d(CONFIG_LAST_SET, ((config_get_d(CONFIG_LAST_SET)/SET_GROUP)-1)*SET_GROUP);
	return goto_state(&st_set);
    
    case SET_NEXT:
	config_set_d(CONFIG_LAST_SET, ((config_get_d(CONFIG_LAST_SET)/SET_GROUP)+1)*SET_GROUP);
	return goto_state(&st_set);
    
    default:
	if (set_exists(i))
	{
	    config_set_d(CONFIG_LAST_SET, i);
	    set_goto(i);
	    return goto_state(&st_start);
	}
    }
    return 1;
}

static int set_enter(void)
{
    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);
    int last_set = config_get_d(CONFIG_LAST_SET);
    int b = last_set / SET_GROUP;
    int i;

    int id, jd, kd;

    set_init();

    audio_music_fade_to(0.5f, "bgm/inter.ogg");
    audio_play(AUD_START, 1.f);

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_hstack(id)))
        {
            gui_label(jd, _("Level Set"), GUI_SML, GUI_ALL, gui_yel, gui_red);
            gui_filler(jd);
	    if (set_exists((b+1)*SET_GROUP))
	        gui_state(jd, _("Next"), GUI_SML, SET_NEXT, 0);
	    else
		gui_label(jd, _("Next"), GUI_SML, GUI_ALL, gui_gry, gui_gry);

            gui_state(jd, _("Back"),  GUI_SML, SET_BACK, 0);
	    
	    if (b>0)
		gui_state(jd, _("Prev"), GUI_SML, SET_PREV, 0);
	    else
		gui_label(jd, _("Prev"), GUI_SML, GUI_ALL, gui_gry, gui_gry);
        }

        if ((jd = gui_harray(id)))
        {
            shot_id = gui_image(jd, set_shot(last_set), 7 * w / 16, 7 * h / 16);

            if ((kd = gui_varray(jd)))
	    {
		/* Display levels */
	        for(i=b*SET_GROUP; i<(b+1)*SET_GROUP && set_exists(i); i++)
		{
		    if(last_set == i)
		        gui_start(kd, _(set_name(i)), GUI_SML, i, 0);
		    else
		        gui_state(kd, _(set_name(i)), GUI_SML, i, 0);
		}
		
		/* Display empty slots */
		for(; i<(b+1)*SET_GROUP; i++)
		    gui_filler(kd);
	    }	       
        }

        gui_space(id);
        desc_id = gui_multi(id, " \\ \\ \\ \\ \\", GUI_SML, GUI_ALL, gui_yel, gui_wht);

        gui_layout(id, 0, 0);
    }
    return id;
}

static void set_over(int i)
{
    gui_set_image(shot_id, set_shot(i));
    gui_set_multi(desc_id, _(set_desc(i)));
}

static void set_point(int id, int x, int y, int dx, int dy)
{
    int jd = shared_point_basic(id, x, y);
    int i  = gui_token(jd);
    if (jd && set_exists(i))
	set_over(i);
}

static void set_stick(int id, int a, int v)
{
    int jd = shared_stick_basic(id, a, v);
    int i  = gui_token(jd);
    if (jd && set_exists(i))
	set_over(i);
}

static int set_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return set_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_title);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_set = {
    set_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    set_point,
    set_stick,
    shared_click,
    NULL,
    set_buttn,
    1, 0
};
