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

#ifndef HUD_H
#define HUD_H

#include "SDL_events.h"

/*---------------------------------------------------------------------------*/

enum
{
    PUTT_TOUCH_NONE = 0,
    PUTT_TOUCH_PAUSE = 1
};

void putt_hud_init(void);
void putt_hud_free(void);

void putt_hud_paint(void);

int  putt_hud_touch(const SDL_TouchFingerEvent *event);
void putt_hud_touch_timer(float dt);
void putt_hud_touch_reset(void);

void putt_hud_joystick_set(int active, float ox, float oy, float kx, float ky, float rb, float rk);

/*---------------------------------------------------------------------------*/

#endif
