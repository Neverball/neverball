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

extern "C" {
#include "config.h"
}

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
    static bool btn_l1 = 0;
    static bool btn_r1 = 0;
    static bool btn_start = 0;

    HandList hands = leapctrl.frame().hands();
    Hand hand;
    Finger thumb;
    float angle;
    bool cond;

    if (!tilt_stat() || hands.count() != 1)
    {
        return 0;
    }

    hand = hands.frontmost();
    thumb = hand.fingers().fingerType(Finger::TYPE_THUMB)[0];
    angle = hand.direction().angleTo(thumb.direction()) * 180.0f / 3.14159f;

    if ((cond = (angle < 20.0f && hand.isRight()) ||
                (angle > 55.0f && hand.isLeft())) != btn_l1)
    {
        *b = config_get_d(CONFIG_JOYSTICK_BUTTON_L1);
        return (*s = btn_l1 = cond) + 1;
    }
    else if ((cond = (angle > 55.0f && hand.isRight()) ||
                     (angle < 20.0f && hand.isLeft())) != btn_r1)
    {
        *b = config_get_d(CONFIG_JOYSTICK_BUTTON_R1);
        return (*s = btn_r1 = cond) + 1;
    }
    else if ((cond = hand.grabStrength() == 1.0f) != btn_start)
    {
        *b = config_get_d(CONFIG_JOYSTICK_BUTTON_START);
        return (*s = btn_start = cond) + 1;
    }

    return 0;
}

float tilt_get_x(void)
{
    HandList hands = leapctrl.frame().hands();

    if (!tilt_stat() || hands.count() != 1)
    {
        return 0.0f;
    }

    return -hands.frontmost().direction().pitch() * 180.0f / 3.14159f + 10.0f;
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

