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

#include "checkpoints.h"

/*---------------------------------------------------------------------------*/

int last_active;

int last_position_x;
int last_position_y;
int last_position_z;
int last_r;

int last_time;
int last_coins;
int last_goal;

int last_timer_down;

/*---------------------------------------------------------------------------*/

void set_active_checkpoint(int new_active)
{
    last_active = new_active;
}

void set_last_transform(float x, float y, float z, float radius)
{
    /* Set the last transform */
    last_pos_x = x;
    last_pos_y = y;
    last_pos_z = z;
    last_r = radius;
}

void set_last_data(int time, int coins, int goal)
{
    /* Set the last level data */
    last_time = time;
    last_coins = coins;
    last_goal = goal;
}

void set_last_timer_down(int enabled)
{
    /* Set the timer downward */
    last_timer_down = enabled;
}
