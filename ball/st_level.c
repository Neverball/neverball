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
#include "game.h"
#include "set.h"
#include "level.h"
#include "audio.h"
#include "config.h"
#include "st_shared.h"

#include "st_level.h"
#include "st_play.h"
#include "st_start.h"
#include "st_over.h"

/*---------------------------------------------------------------------------*/

static int level_enter(void)
{
    int id, jd, kd, ld;
    const char * ln;

    /* Load the level */
    level_play_go();
    
    ln = _(level_number_name(curr_level()));
	    
    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_hstack(id)))
        {
            gui_filler(jd);
	    if ((kd = gui_vstack(jd)))
	    {
		if (level_extra_bonus(curr_level()))
		    gui_label(kd, _("*** BONUS ***"),  GUI_MED, GUI_TOP, gui_wht, gui_grn);
		if ((ld = gui_hstack(kd)))
		{
		    if (level_extra_bonus(curr_level()))
		    {
		        gui_label(ld, ln,          GUI_LRG, 0, gui_wht, gui_grn);
		        gui_label(ld, _("Level "), GUI_LRG, 0, gui_wht, gui_grn);
		    }
		    else
		    {
		        gui_label(ld, ln,          GUI_LRG, GUI_NE, 0, 0);
		        gui_label(ld, _("Level "), GUI_LRG, GUI_NW, 0, 0);
		    }
		}
		gui_label(kd, _(set_name(set_curr())),  GUI_SML, GUI_BOT, gui_wht, gui_wht);
	    }
            gui_filler(jd);
        }
        gui_space(id);
        gui_multi(id, _(curr_intro()), GUI_SML, GUI_ALL, gui_wht, gui_wht);

        gui_layout(id, 0, 0);
    }

    game_set_fly(1.f);

    return id;
}

static void level_timer(int id, float dt)
{
    game_step_fade(dt);
    audio_timer(dt);
}

static int level_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_play_ready) : 1;
}

static int level_keybd(int c, int d)
{
    if (d && c == SDLK_F12)
        return goto_state(&st_poser);
    return 1;
}

static int level_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_play_ready);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
	{
	    if (level_mode() == MODE_CHALLENGE)
		return goto_state(&st_over);
	    else
		return goto_state(&st_start);
	}
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void poser_paint(int id, float st)
{
    game_draw(1, st);
}

static int poser_buttn(int c, int d)
{
    return (d && config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, c)) ? goto_state(&st_level) : 1;
}

/*---------------------------------------------------------------------------*/

struct state st_level = {
    level_enter,
    shared_leave,
    shared_paint,
    level_timer,
    NULL,
    NULL,
    level_click,
    level_keybd,
    level_buttn,
    1, 0
};

struct state st_poser = {
    NULL,
    NULL,
    poser_paint,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    poser_buttn,
    1, 0
};
