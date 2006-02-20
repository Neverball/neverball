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

#include <SDL.h>
#include <math.h>
#include <string.h>

#include "glext.h"
#include "hud.h"
#include "gui.h"
#include "game.h"
#include "level.h"
#include "config.h"

/*---------------------------------------------------------------------------*/

static int Lhud_id;
static int Rhud_id;
static int Rhud2_id;
static int time_id;

static int coin_id;
static int coin2_id;
static int ball_id;
static int scor_id;
static int goal_id;
static int view_id;
static int fps_id;

static float view_timer;

static void hud_fps(void)
{
    static int fps   = 0;
    static int then  = 0;
    static int count = 0;

    int now = SDL_GetTicks();

    if (now - then > 250)
    {
        fps   = count * 1000 / (now - then);
        then  = now;
        count = 0;

        gui_set_count(fps_id, fps);
    }
    else count++;
}

void hud_init(void)
{
    int id;
    char * str_view;

    if ((Rhud_id = gui_hstack(0)))
    {
        if ((id = gui_vstack(Rhud_id)))
        {
            gui_label(id, _("Coins"), GUI_SML, 0, gui_wht, gui_wht);
            gui_label(id, _("Goal"),  GUI_SML, 0, gui_wht, gui_wht);
        }
        if ((id = gui_vstack(Rhud_id)))
        {
            coin_id = gui_count(id, 100, GUI_SML, GUI_NW);
            goal_id = gui_count(id, 10,  GUI_SML, 0);
        }
        gui_layout(Rhud_id, +1, -1);
    }
    
    if ((Rhud2_id = gui_hstack(0)))
    {
        gui_label(Rhud2_id, _("Coins"), GUI_SML, 0, gui_wht, gui_wht);
        coin2_id = gui_count(Rhud2_id, 100,   GUI_SML, GUI_NW);
        gui_layout(Rhud2_id, +1, -1);
    }

    if ((Lhud_id = gui_hstack(0)))
    {
        if ((id = gui_vstack(Lhud_id)))
        {
            ball_id = gui_count(id, 10,   GUI_SML, GUI_NE);
            scor_id = gui_count(id, 1000, GUI_SML, 0);
        }
        if ((id = gui_vstack(Lhud_id)))
        {
            gui_label(id, _("Balls"), GUI_SML, 0, gui_wht, gui_wht);
            gui_label(id, _("Score"), GUI_SML, 0, gui_wht, gui_wht);
        }
        gui_layout(Lhud_id, -1, -1);
    }

    if ((time_id = gui_clock(0, 59999, GUI_MED, GUI_TOP)))
        gui_layout(time_id, 0, -1);

    str_view = strlen(STR_VIEW0) > strlen(STR_VIEW1) ? STR_VIEW0 : STR_VIEW1;
    if (strlen(str_view) < strlen(STR_VIEW2))
	str_view = STR_VIEW2;
    if ((view_id = gui_label(0, str_view, GUI_SML, GUI_SW, gui_wht, gui_wht)))
        gui_layout(view_id, 1, 1);

    if ((fps_id = gui_count(0, 1000, GUI_SML, GUI_SE)))
        gui_layout(fps_id, -1, 1);
}

void hud_free(void)
{
    gui_delete(Rhud_id);
    gui_delete(Lhud_id);
    gui_delete(time_id);
    gui_delete(view_id);
    gui_delete(fps_id);
}

void hud_paint(void)
{
    int mode = level_mode();
    if (mode == MODE_CHALLENGE)
        gui_paint(Lhud_id);
    if (mode == MODE_FREE)
	gui_paint(Rhud2_id);
    else
        gui_paint(Rhud_id);
    gui_paint(time_id);

    if (config_get_d(CONFIG_FPS))
        gui_paint(fps_id);

    if (view_timer > 0.0f)
        gui_paint(view_id);
}

void hud_timer(float dt)
{
    const int clock = curr_clock();
    const int balls = curr_balls();
    const int coins = curr_coins();
    const int score = curr_score();
    const int goal  = curr_goal();
    int mode = level_mode();

    if (gui_value(time_id) != clock) gui_set_clock(time_id, clock);
    if (mode == MODE_CHALLENGE)
    {
        if (gui_value(ball_id) != balls) gui_set_count(ball_id, balls);
	if (gui_value(scor_id) != score) gui_set_count(scor_id, score);
    }
    if (gui_value(coin_id) != coins)
    {
        if (mode == MODE_FREE)
	    gui_set_count(coin2_id, coins);
	else
	    gui_set_count(coin_id, coins);
    }
    if (gui_value(goal_id) != goal)  gui_set_count(goal_id, goal);

    if (config_get_d(CONFIG_FPS))
        hud_fps();

    view_timer -= dt;

    gui_timer(Rhud_id, dt);
    gui_timer(Lhud_id, dt);
    gui_timer(time_id, dt);
    gui_timer(view_id, dt);
}

void hud_ball_pulse(float k) { gui_pulse(ball_id, k); }
void hud_time_pulse(float k) { gui_pulse(time_id, k); }
void hud_coin_pulse(float k) { gui_pulse(coin_id, k); }
void hud_goal_pulse(float k) { gui_pulse(goal_id, k); }

void hud_view_pulse(int c)
{
    switch (c)
    {   
    case 0: gui_set_label(view_id, STR_VIEW0); break;
    case 1: gui_set_label(view_id, STR_VIEW1); break;
    case 2: gui_set_label(view_id, STR_VIEW2); break;
    }

    gui_pulse(view_id, 1.2f);
    view_timer = 2.0f;
}

/*---------------------------------------------------------------------------*/
