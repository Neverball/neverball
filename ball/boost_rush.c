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

#include "boost_rush.h"

/*---------------------------------------------------------------------------*/

int need_coins;
float curr_speed;
float sonic_timer;

void collect_coins(int add)
{
    need_coins += add;
    while (need_coins > 9)
    {
        if (curr_speed >= 100.f)
            /* Exeed some speed for 5 seconds */
            sonic_timer += 5.f;
        else
            /* Increase the speed */
            increase_speed();
        
        need_coins = 0;
    }
}

void increase_speed(void)
{
    curr_speed += 14.3f;
    
    if (curr_speed >= 100.f)
    {
        curr_speed = 100.f;
    }
}

void substract_speed(void)
{
    curr_speed /= 2f;
}

void speed_timer(float dt)
{
    if (sonic_timer > 0)
    {
        sonic_timer += dt;
    }
}

float get_speed_indicator(void)
{
    return curr_speed;
}

int is_sonic(void)
{
    return (sonic_timer > 0);
}
