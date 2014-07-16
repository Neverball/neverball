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

#include <Leap.h>

using namespace Leap;

static Controller leapctrl;

extern "C" {

void tilt_init(void)
{
    // nothing to do
}

void tilt_free(void)
{
    // nothing to do
}

int tilt_stat(void)
{
    return leapctrl.isConnected();
}

int tilt_get_button(int *b, int *s)
{
    return 0;
}

float tilt_get_x(void)
{
    HandList hands = leapctrl.frame().hands();

    if (!tilt_stat() || hands.count() != 1)
    {
        return 0.0f;
    }

    return -hands.frontmost().direction().pitch() * 180.0f / 3.14159f;
}

float tilt_get_z(void)
{
    HandList hands = leapctrl.frame().hands();

    if (!tilt_stat() || hands.count() != 1)
    {
        return 0.0f;
    }

    return -hands.frontmost().palmNormal().roll() * 180.0f / 3.14159f;
}

}
