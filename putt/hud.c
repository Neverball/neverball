/*
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERPUTT is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

#include "common.h"
#include "gui.h"
#include "hud.h"
#include "hole.h"
#include "config.h"
#include "video.h"
#include "glext.h"
#include "geom.h"
#include "lang.h"

/*---------------------------------------------------------------------------*/

static int Lhud_id;
static int Rhud_id;
static int fps_id;

static int touch_pause_id;
static float touch_timer;

static struct
{
    int active;
    float ox, oy;
    float kx, ky;
    float rb, rk;
} joystick;

/*---------------------------------------------------------------------------*/

void putt_hud_init(void)
{
    static const GLubyte *color[5] = {
        gui_wht,
        gui_red,
        gui_grn,
        gui_blu,
        gui_yel
    };
    int i = curr_player();
    int id;

    if ((Lhud_id = gui_hstack(0)))
    {
        gui_label(Lhud_id, curr_scr(), GUI_MED, color[i], gui_wht);
        gui_label(Lhud_id, _("Score"), GUI_SML,  gui_wht,  gui_wht);
        gui_set_rect(Lhud_id, GUI_NE);
        gui_layout(Lhud_id, -1, -1);
    }
    if ((Rhud_id = gui_hstack(0)))
    {
        gui_label(Rhud_id, curr_par(), GUI_MED,  color[i], gui_wht);
        gui_label(Rhud_id, _("Par"),   GUI_SML, gui_wht,  gui_wht);
        gui_set_rect(Rhud_id, GUI_NW);
        gui_layout(Rhud_id, +1, -1);
    }
    if ((fps_id = gui_count(0, 1000, GUI_SML)))
    {
        gui_set_rect(fps_id, GUI_SE);
        gui_layout(fps_id, -1, +1);
    }

    int mode = config_get_d(CONFIG_TOUCH_MODE);
    int side = (mode == TOUCH_MODE_RL) ? -1 : +1;

    if ((touch_pause_id = gui_vstack(0)))
    {
        gui_space(touch_pause_id);
        if ((id = gui_hstack(touch_pause_id)))
        {
            gui_space(id);
            gui_state(id, GUI_ROMAN_2, GUI_TCH, PUTT_TOUCH_PAUSE, 0);
            gui_space(id);
        }
        gui_set_slide(touch_pause_id, GUI_N, 0, 0, 0);
        gui_layout(touch_pause_id, side, +1);
    }

    touch_timer = 0.0f;
    joystick.active = 0;
}

void putt_hud_free(void)
{
    gui_delete(Lhud_id);
    gui_delete(Rhud_id);
    gui_delete(fps_id);
    gui_delete(touch_pause_id);

    Lhud_id = 0;
    Rhud_id = 0;
    fps_id = 0;
    touch_pause_id = 0;
    touch_timer = 0.0f;
    joystick.active = 0;
}

/*---------------------------------------------------------------------------*/

int putt_hud_touch(const SDL_TouchFingerEvent *event)
{
    const int x = (int) ((float) video.device_w * event->x);
    const int y = (int) ((float) video.device_h * (1.0f - event->y));

    if (touch_timer <= 0.0f)
        gui_slide(touch_pause_id, GUI_N | GUI_EASE_BACK, 0, 0.3f, 0);
    touch_timer = 5.0f;

    int hit_pause = gui_search(touch_pause_id, x, y);
    if (hit_pause)
    {
        if (event->type == SDL_FINGERUP)
        {
            gui_focus(0);
            return PUTT_TOUCH_PAUSE;
        }
        return touch_pause_id;
    }

    return 0;
}

void putt_hud_touch_timer(float dt)
{
    if (touch_timer > 0.0f)
    {
        touch_timer -= dt;
        if (touch_timer <= 0.0f)
        {
            touch_timer = 0.0f;
            gui_slide(touch_pause_id, GUI_N | GUI_EASE_BACK | GUI_BACKWARD, 0, 0.3f, 0);
        }
    }

    gui_timer(touch_pause_id, dt);
}

void putt_hud_touch_reset(void)
{
    touch_timer = 0.0f;
    joystick.active = 0;
    gui_set_slide(touch_pause_id, GUI_N, 0, 0, 0);
}

void putt_hud_joystick_set(int active, float ox, float oy, float kx, float ky, float rb, float rk)
{
    joystick.active = active;
    joystick.ox = ox;
    joystick.oy = oy;
    joystick.kx = kx;
    joystick.ky = ky;
    joystick.rb = rb;
    joystick.rk = rk;
}

/*---------------------------------------------------------------------------*/

void putt_hud_paint(void)
{
    if (config_get_d(CONFIG_FPS))
    {
        gui_set_count(fps_id, video_perf());
        gui_paint(fps_id);
    }

    gui_paint(Rhud_id);
    gui_paint(Lhud_id);
    gui_paint(touch_pause_id);

    if (joystick.active)
        joystick_draw(joystick.ox, joystick.oy,
                      joystick.kx, joystick.ky,
                      joystick.rb, joystick.rk);
}

/*---------------------------------------------------------------------------*/
