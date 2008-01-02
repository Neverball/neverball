/*
 * Copyright (C) 2003 Robert Kooima - 2006 Jean Privat
 * Part of the Neverball Project http://icculus.org/neverball/
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


#include <string.h>

#include "gui.h"
#include "back.h"
#include "geom.h"
#include "part.h"
#include "audio.h"
#include "config.h"

#include "st_resol.h"

extern struct state st_conf;
extern struct state st_null;

static SDL_Rect **modes;

/*---------------------------------------------------------------------------*/

#define RESOL_BACK -1

static int resol_action(int i)
{
    int r = 1;

    switch (i)
    {
    case RESOL_BACK:
        goto_state(&st_conf);
        break;

    default:
        goto_state(&st_null);
        r = config_mode(config_get_d(CONFIG_FULLSCREEN),
                        modes[i]->w, modes[i]->h);
        goto_state(&st_resol);
        break;
    }

    return r;
}

static int resol_enter(void)
{
    int id, jd;

    back_init("back/gui.png", config_get_d(CONFIG_GEOMETRY));

    modes = SDL_ListModes(NULL, SDL_OPENGL | SDL_FULLSCREEN);

    if ((int) modes == -1)
        modes = NULL;

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_harray(id)))
        {
            gui_label(jd, _("Resolution"), GUI_SML, GUI_ALL, 0, 0);
            gui_space(jd);
            gui_start(jd, _("Back"),       GUI_SML, RESOL_BACK, 0);
        }

        gui_space(id);

        if (modes)
        {
            int i;

            for (i = 0; modes[i]; i++)
            {
                char s[20];

                sprintf(s, "%d x %d", modes[i]->w, modes[i]->h);

                if (i % 4 == 0)
                    jd = gui_harray(id);

                gui_state(jd, s, GUI_SML, i,
                          config_get_d(CONFIG_WIDTH)  == modes[i]->w &&
                          config_get_d(CONFIG_HEIGHT) == modes[i]->h);
            }

            for(; i % 4 != 0; i++)
                gui_space(jd);
        }

        gui_layout(id, 0, 0);
    }

    audio_music_fade_to(0.5f, "bgm/inter.ogg");

    return id;
}

static void resol_leave(int id)
{
    back_free();
    gui_delete(id);
}

static void resol_paint(int id, float st)
{
    config_push_persp((float) config_get_d(CONFIG_VIEW_FOV), 0.1f, FAR_DIST);
    {
        back_draw(0);
    }
    config_pop_matrix();
    gui_paint(id);
}

static void resol_timer(int id, float dt)
{
    gui_timer(id, dt);
}

static void resol_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void resol_stick(int id, int a, int v)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int resol_click(int b, int d)
{
    if (b < 0 && d == 1)
        return resol_action(gui_token(gui_click()));
    return 1;
}

static int resol_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_conf) : 1;
}

static int resol_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return resol_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_conf);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_conf);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/


struct state st_resol = {
    resol_enter,
    resol_leave,
    resol_paint,
    resol_timer,
    resol_point,
    resol_stick,
    NULL,
    resol_click,
    resol_keybd,
    resol_buttn,
    1, 0
};

