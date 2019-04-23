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

/*
 * Checkpoints discovered by: PennySchloss
 * Thanks for the Youtuber: PlayingWithMahWii
 */
#include "checkpoints.h"

#include "game_server.h"

/*---------------------------------------------------------------------------*/

int checkpoints_busy;

float current_game_step;
float saved_game_step;

int last_active;

/* Transforms */
float last_position_x;
float last_position_y;
float last_position_z;
float last_orientation_x;
float last_orientation_y;
float last_orientation_z;
float last_r;

/* Level datas */
float last_time;
int last_coins;
int last_goal;

int last_timer_down;

/*---------------------------------------------------------------------------*/

void checkpoints_increment_server_step(float dt)
{
    current_game_step += dt;
}

void checkpoints_save_game_server_step(void)
{
    saved_game_step = current_game_step;
}

void checkpoints_respawn(void)
{
    checkpoints_busy = 1;
    game_server_step(saved_game_step);
}

void checkpoints_respawn_done(void)
{
    checkpoints_busy = 0;
}

void checkpoints_stop(void)
{
    current_game_step = 0;
    saved_game_step = 0;

    last_active = 0;

    last_position_x = 0;
    last_position_y = 0;
    last_position_z = 0;
    last_r = 1.f;

    last_time = 0;
    last_coins = 0;
    last_goal = 0;

    last_timer_down = 0;
}

void set_active_checkpoint(int new_active)
{
    last_active = new_active;
}

void set_last_transform(float pos[3], float radius)
{
    /* Set the last transform */
    last_position_x = pos[0];
    last_position_y = pos[1];
    last_position_z = pos[2];
    last_r = radius;
}

void set_last_data(float time, int coins)
{
    /* Set the last level data */
    last_time = time;
    last_coins = coins;
}

void set_last_timer_down(int enabled)
{
    /* Set the timer downward */
    last_timer_down = enabled;
}
