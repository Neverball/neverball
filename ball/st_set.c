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

#include "st_title.h"
#include "st_start.h"

/*---------------------------------------------------------------------------*/

#define SET_BACK -1

static int shot_id;
static int desc_id;

static int set_action(int i)
{
    audio_play(AUD_MENU, 1.0f);
    
    if (i == SET_BACK)
        return goto_state(&st_title);

    if (0 <= i && i <= 5)
    {
        set_goto(i);
        return goto_state(&st_start);
    }
    return 1;
}

static void gui_set(int id, int i)
{
    if (set_exists(i))
        gui_state(id, set_name(i), GUI_SML, i, 0);
    else
        gui_label(id, "", GUI_SML, GUI_ALL, 0, 0);
}

static int set_enter(void)
{
    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);

    int id, jd, kd;

    set_init();

    audio_music_fade_to(0.5f, "bgm/inter.ogg");
    audio_play(AUD_START, 1.f);

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_harray(id)))
        {
            gui_label(jd, "Level Set", GUI_SML, GUI_ALL, gui_yel, gui_red);
            gui_filler(jd);
            gui_filler(jd);
            gui_start(jd, "Back",  GUI_SML, SET_BACK, 0);
        }

        if ((jd = gui_harray(id)))
        {
            shot_id = gui_image(jd, "shot-rlk/easy.jpg", 7 * w / 16, 7 * h / 16);

            if ((kd = gui_varray(jd)))
            {
                gui_set(kd, 0);
                gui_set(kd, 1);
                gui_set(kd, 2);
                gui_set(kd, 3);
                gui_set(kd, 4);
            }
        }

        gui_space(id);
        desc_id = gui_multi(id, " \\ \\ \\ \\ \\", GUI_SML, GUI_ALL, gui_yel, gui_wht);

        gui_layout(id, 0, 0);
    }
    return id;
}

static void set_leave(int id)
{
    gui_delete(id);
}

static void set_paint(int id, float st)
{
    game_draw(0, st);
    config_pop_matrix();
    gui_paint(id);
}

static void set_timer(int id, float dt)
{
    gui_timer(id, dt);
    audio_timer(dt);
}

static void set_point(int id, int x, int y, int dx, int dy)
{
    int jd;

    if ((jd = gui_point(id, x, y)))
    {
        int i = gui_token(jd);

        gui_set_image(shot_id, set_shot(i));
        gui_set_multi(desc_id, set_desc(i));
        gui_pulse(jd, 1.2f);
    }
}

static void set_stick(int id, int a, int v)
{
    int jd;

    int x = (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a)) ? v : 0;
    int y = (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a)) ? v : 0;

    if ((jd = gui_stick(id, x, y)))
    {
        int i = gui_token(jd);

        gui_set_image(shot_id, set_shot(i));
        gui_set_multi(desc_id, set_desc(i));
        gui_pulse(jd, 1.2f);
    }
}

static int set_click(int b, int d)
{
    return (b < 0 && d == 1) ? set_action(gui_token(gui_click())) : 1;
}

static int set_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_title) : 1;
}

static int set_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return set_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_title);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_title);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_set = {
    set_enter,
    set_leave,
    set_paint,
    set_timer,
    set_point,
    set_stick,
    set_click,
    set_keybd,
    set_buttn,
    1, 0
};
