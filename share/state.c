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

#include "glext.h"
#include "state.h"
#include "config.h"
#include "video.h"
#include "common.h"

/*---------------------------------------------------------------------------*/

static float         state_time;
static int           state_drawn;
static struct state *state;

struct state *curr_state(void)
{
    return state;
}

float time_state(void)
{
    return state_time;
}

void init_state(struct state *st)
{
    state = st;
}

int goto_state(struct state *st)
{
    struct state *prev = state;

    if (state && state->leave)
        state->leave(state, st, state->gui_id);

    state       = st;
    state_time  = 0;
    state_drawn = 0;

    if (state && state->enter)
        state->gui_id = state->enter(state, prev);

    return 1;
}

/*---------------------------------------------------------------------------*/

void st_paint(float t)
{
    int stereo = config_get_d(CONFIG_STEREO);

    state_drawn = 1;

    if (state && state->paint)
    {
        if (stereo)
        {
            glDrawBuffer(GL_BACK_LEFT);
            video_clear();
            state->paint(state->gui_id, t);

            glDrawBuffer(GL_BACK_RIGHT);
            video_clear();
            state->paint(state->gui_id, t);
        }
        else
        {
            video_clear();
            state->paint(state->gui_id, t);
        }
    }
}

void st_timer(float dt)
{
    if (!state_drawn)
        return;

    state_time += dt;

    if (state && state->timer)
        state->timer(state->gui_id, dt);
}

void st_point(int x, int y, int dx, int dy)
{
    if (state && state->point)
        state->point(state->gui_id, x, y, dx, dy);
}

int STICK_BUMP;

void st_stick(int a, float v)
{
    static struct
    {
        const int *num;
        const int *inv;

        float prev;
    } axes[] = {
        { &CONFIG_JOYSTICK_AXIS_X, &CONFIG_JOYSTICK_AXIS_X_INVERT },
        { &CONFIG_JOYSTICK_AXIS_Y, &CONFIG_JOYSTICK_AXIS_Y_INVERT },
        { &CONFIG_JOYSTICK_AXIS_U, &CONFIG_JOYSTICK_AXIS_U_INVERT }
    };

    int i;

    STICK_BUMP = 0;

    for (i = 0; i < ARRAYSIZE(axes); i++)
        if (config_tst_d(*axes[i].num, a))
        {
            float p = axes[i].prev;

            /* Note the transition from centered to leaned position. */

            STICK_BUMP = ((-0.5f <= p && p <= +0.5f) &&
                          (v < -0.5f || +0.5f < v));

            axes[i].prev = v;

            if (config_get_d(*axes[i].inv))
                v = -v;

            break;
        }

    if (state && state->stick)
        state->stick(state->gui_id, a, v);
}

void st_angle(int x, int z)
{
    if (state && state->angle)
        state->angle(state->gui_id, x, z);
}

/*---------------------------------------------------------------------------*/

int st_click(int b, int d)
{
    return (state && state->click) ? state->click(b, d) : 1;
}

int st_keybd(int c, int d)
{
    return (state && state->keybd) ? state->keybd(c, d) : 1;
}

int st_buttn(int b, int d)
{
    return (state && state->buttn) ? state->buttn(b, d) : 1;
}

/*---------------------------------------------------------------------------*/
