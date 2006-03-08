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
#include "levels.h"
#include "audio.h"
#include "config.h"
#include "st_shared.h"

#include "st_fail.h"
#include "st_over.h"
#include "st_done.h"
#include "st_save.h"
#include "st_level.h"
#include "st_start.h"

/*---------------------------------------------------------------------------*/

#define FAIL_BACK  0
#define FAIL_OVER  1
#define FAIL_RETRY 2
#define FAIL_SAVE  3
#define FAIL_NEXT  4
#define FAIL_DONE  5

static int fail_action(int i)
{
    struct state *next;
    const struct level_game *lg = curr_lg();
    switch (i)
    {
    case FAIL_BACK:
        return goto_end_level();
	    
    case FAIL_OVER:
        return goto_state(&st_over);

    case FAIL_RETRY:
        return goto_state(&st_level);
	
    case FAIL_DONE:
        return goto_state(&st_over);
	
    case FAIL_NEXT:
	level_next();
        return goto_state(&st_level);

    case FAIL_SAVE:
	if (lg->next_level)
	{
	    level_next();
	    next = &st_level;
	}
	else if (lg->dead)
	    next = &st_over;
	else if (lg->win)
	    next = &st_done;
	else
	    next = &st_level;
		
	return goto_save(next, next);
    }
    return 1;
}

static int fail_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return fail_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
	    return fail_action(FAIL_BACK);
    }
    return 1;
}

static int gui_fail(const char *title)
{
    int id, jd, kd;
    const struct level_game *lg = curr_lg();

    if ((id = gui_vstack(0)))
    {
        kd = gui_label(id, title, GUI_LRG, GUI_ALL, gui_gry, gui_red);
    
        if ((jd = gui_harray(id)))
        {
            gui_state(jd, _("Save Replay"),     GUI_SML, FAIL_SAVE,  0);

	    if (lg->next_level != NULL && lg->mode != MODE_CHALLENGE)
                gui_state(jd, _("Retry Level"), GUI_SML, FAIL_RETRY, 0);
	    
            if (lg->dead)
                gui_start(jd, _("Game Over"),   GUI_SML, FAIL_OVER,  0);
	    else if (lg->win)
                gui_start(jd, _("Finish"),        GUI_SML, FAIL_DONE,  0);
	    else if (lg->next_level != NULL)
                gui_start(jd, _("Next Level"),  GUI_SML, FAIL_NEXT,  0);
	    else
                gui_start(jd, _("Retry Level"), GUI_SML, FAIL_RETRY, 0);
        }

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);
    }

    return id;
}

/*---------------------------------------------------------------------------*/

static int fall_out_enter(void)
{
    int id = gui_fail(_("Fall-out!"));

    audio_music_fade_out(2.0f);
    audio_play(AUD_FALL, 1.0f);

    config_clr_grab();

    return id;
}

static void fall_out_timer(int id, float dt)
{
    float g[3] = { 0.0f, -9.8f, 0.0f };

    if (time_state() < 2.f)
        game_step(g, dt, NULL);

    gui_timer(id, dt);
    audio_timer(dt);
}


/*---------------------------------------------------------------------------*/

static int time_out_enter(void)
{
    int id = gui_fail(_("Time's Up!"));

    audio_music_fade_out(2.0f);
    audio_play(AUD_TIME, 1.0f);

    config_clr_grab();

    return id;
}

/*---------------------------------------------------------------------------*/

struct state st_fall_out = {
    fall_out_enter,
    shared_leave,
    shared_paint,
    fall_out_timer,
    shared_point,
    shared_stick,
    shared_click,
    NULL,
    fail_buttn,
    1, 0
};

struct state st_time_out = {
    time_out_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_click,
    NULL,
    fail_buttn,
    1, 0
};
