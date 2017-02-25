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

#include "gui.h"
#include "config.h"
#include "audio.h"
#include "state.h"

#include "game_server.h"
#include "game_client.h"

#include "st_shared.h"

void shared_leave(struct state *st, struct state *next, int id)
{
    gui_delete(id);
}

void shared_paint(int id, float t)
{
    game_client_draw(0, t);
    gui_paint(id);
}

void shared_timer(int id, float dt)
{
    gui_timer(id, dt);
}

int shared_point_basic(int id, int x, int y)
{
    int jd;

    if ((jd = gui_point(id, x, y)))
        gui_pulse(jd, 1.2f);

    return jd;
}

void shared_point(int id, int x, int y, int dx, int dy)
{
    shared_point_basic(id, x, y);
}

int shared_stick_basic(int id, int a, float v, int bump)
{
    int jd;

    if ((jd = gui_stick(id, a, v, bump)))
        gui_pulse(jd, 1.2f);

    return jd;
}

void shared_stick(int id, int a, float v, int bump)
{
    shared_stick_basic(id, a, v, bump);
}

void shared_angle(int id, float x, float z)
{
    game_set_ang(x, z);
}

int shared_click_basic(int b, int d)
{
    /* Activate on left click. */

    if (b == SDL_BUTTON_LEFT && d)
        return st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 1);
    else
        return 1;
}

int shared_click(int b, int d)
{
    /* Activate based on GUI state. */

    if (gui_click(b, d))
        return st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 1);
    else
        return 1;
}

