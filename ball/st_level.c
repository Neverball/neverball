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
#include "level.h"
#include "audio.h"
#include "config.h"

#include "st_level.h"
#include "st_play.h"
#include "st_over.h"

/*---------------------------------------------------------------------------*/

static int level_enter(void)
{
    int id, jd;

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_hstack(id)))
        {
            gui_filler(jd);
            gui_count(jd, curr_level(), GUI_LRG, GUI_NE | GUI_SE);
            gui_label(jd, "Level ",     GUI_LRG, GUI_NW | GUI_SW, 0, 0);
            gui_filler(jd);
        }
        gui_space(id);
        gui_multi(id, curr_intro(), GUI_SML, GUI_ALL, gui_wht, gui_wht);

        gui_layout(id, 0, 0);
    }

    game_set_fly(1.f);

    return id;
}

static void level_leave(int id)
{
    gui_delete(id);
}

static void level_timer(int id, float dt)
{
    game_step_fade(dt);
    audio_timer(dt);
}

static void level_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static int level_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_play_ready) : 1;
}

static int level_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        goto_state(&st_over);
    if (d && c == SDLK_F12)
        goto_state(&st_poser);
    return 1;
}

static int level_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_play_ready);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void poser_paint(int id, float st)
{
    game_draw(1, st);
}

static int poser_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_level) : 1;
}

/*---------------------------------------------------------------------------*/

struct state st_level = {
    level_enter,
    level_leave,
    level_paint,
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
    poser_keybd,
    NULL,
    1, 0
};
