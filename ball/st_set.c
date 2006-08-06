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
#include "levels.h"
#include "game.h"
#include "audio.h"
#include "config.h"
#include "util.h"
#include "st_shared.h"

#include "st_set.h"
#include "st_title.h"
#include "st_start.h"

/*---------------------------------------------------------------------------*/

#define SET_GROUP 5 /* number of sets in one screen */

static int last_set = 0;

static int shot_id;
static int desc_id;

static int set_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case GUI_BACK:
        return goto_state(&st_title);

    case GUI_PREV:
        last_set = (last_set / SET_GROUP - 1) * SET_GROUP;
        return goto_state(&st_set);

    case GUI_NEXT:
        last_set = (last_set / SET_GROUP + 1) * SET_GROUP;
        return goto_state(&st_set);

    case GUI_NULL:
        return 1;

    default:
        if (set_exists(i))
        {
            last_set = i;
            set_goto(i);
            return goto_state(&st_start);
        }
    }
    return 1;
}

static void gui_set(int id, int i)
{
    const struct set *s = get_set(i);
    int jd;

    if (set_completed(s))
        jd = gui_label(id, _(s->name), GUI_SML, GUI_ALL, gui_yel, gui_wht);
    else if (set_unlocked(s))
        jd = gui_label(id, _(s->name), GUI_SML, GUI_ALL, gui_grn, gui_wht);
    else
        jd = gui_label(id, _(s->name), GUI_SML, GUI_ALL, gui_wht, gui_wht);

    gui_active(jd, i, 0);
}

static int set_enter(void)
{
    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);
    int b = last_set / SET_GROUP;
    int i;

    int id, jd, kd;

    set_init();

    /* Reset last set if it does not exist */
    if (!set_exists(last_set))
    {
        b = 0;
        last_set = 0;
    }

    audio_music_fade_to(0.5f, "bgm/inter.ogg");
    audio_play(AUD_START, 1.f);

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_hstack(id)))
        {
            gui_label(jd, _("Level Set"), GUI_SML, GUI_ALL, gui_yel, gui_red);
            gui_filler(jd);
            gui_back_prev_next(jd, b > 0, set_exists((b + 1) * SET_GROUP));
        }

        if ((jd = gui_harray(id)))
        {
            shot_id = gui_image(jd, get_set(last_set)->shot,
                                7 * w / 16, 7 * h / 16);

            if ((kd = gui_varray(jd)))
            {
                /* Display sets */
                for (i = b * SET_GROUP; i < (b + 1) * SET_GROUP && set_exists(i); i++)
                    gui_set(kd, i);

                /* Display empty slots */
                for(; i < (b + 1) * SET_GROUP; i++)
                    gui_filler(kd);
            }
        }

        gui_space(id);
        desc_id = gui_multi(id, " \\ \\ \\ \\ \\", GUI_SML, GUI_ALL,
                            gui_yel, gui_wht);

        gui_layout(id, 0, 0);
    }
    return id;
}

static void set_over(int i)
{
    gui_set_image(shot_id, get_set(i)->shot);
    gui_set_multi(desc_id, _(get_set(i)->desc));
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
            return set_action(GUI_BACK);
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
