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

#include <SDL.h>
#include <SDL_thread.h>
#include <math.h>
#include <stdio.h>

#include "config.h"

/*---------------------------------------------------------------------------*/
#ifdef ENABLE_WII

#define _ENABLE_TILT
#include <libcwiimote/wiimote.h>
#include <libcwiimote/wiimote_api.h>

struct tilt_state
{
    int   status;
    float x;
    float z;
    int   A, last_A;
    int   B, last_B;
    int   P, last_P;
    int   M, last_M;
    int   H, last_H;
    int   U;
    int   D;
    int   L;
    int   R;
};

static struct tilt_state state;
static SDL_mutex        *mutex  = NULL;
static SDL_Thread       *thread = NULL;

#define FILTER 8

static int tilt_func(void *data)
{
    wiimote_t wiimote = WIIMOTE_INIT;
    char      address[MAXSTR];

    config_get_s(CONFIG_WIIMOTE_ADDR, address, MAXSTR);

    if (strlen(address) > 0)
    {
        if (wiimote_connect(&wiimote, address) < 0)
            fprintf(stderr, "%s\n", wiimote_get_error());
        else
        {
            int running = 1;

            wiimote.mode.bits = WIIMOTE_MODE_ACC;
            wiimote.led.one   = 1;

            SDL_mutexP(mutex);
            state.status = running;
            SDL_mutexV(mutex);

            while (mutex && running && wiimote_is_open(&wiimote))
            {
                if (wiimote_update(&wiimote) < 0)
                    break;

                SDL_mutexP(mutex);
                {
                    running = state.status;

                    state.A = wiimote.keys.a;
                    state.B = wiimote.keys.b;
                    state.U = wiimote.keys.up;
                    state.P = wiimote.keys.plus;
                    state.M = wiimote.keys.minus;
                    state.H = wiimote.keys.home;
                    state.D = wiimote.keys.down;
                    state.L = wiimote.keys.left;
                    state.R = wiimote.keys.right;

                    if (isnormal(wiimote.tilt.y))
                    {
                        state.x = (state.x * (FILTER - 1) +
                                   wiimote.tilt.y) / FILTER;
                    }
                    if (isnormal(wiimote.tilt.x))
                    {
                        state.z = (state.z * (FILTER - 1) +
                                   wiimote.tilt.x) / FILTER;
                    }
                }
                SDL_mutexV(mutex);
            }

            wiimote_disconnect(&wiimote);
        }
    }
    return 0;
}

void tilt_init(void)
{
    memset(&state, 0, sizeof (struct tilt_state));

    mutex  = SDL_CreateMutex();
    thread = SDL_CreateThread(tilt_func, NULL);
}

void tilt_free(void)
{
    int b = 0;

    if (mutex)
    {
        /* Get/set the status of the tilt sensor thread. */

        SDL_mutexP(mutex);
        b = state.status;
        state.status = 0;
        SDL_mutexV(mutex);

        /* Kill the thread and destroy the mutex. */

        SDL_KillThread(thread);
        SDL_DestroyMutex(mutex);

        mutex  = NULL;
        thread = NULL;
    }
}

int tilt_get_button(int *b, int *s)
{
    int ch = 0;

    if (mutex)
    {
        SDL_mutexP(mutex);
        {
            if      (state.A != state.last_A)
            {
                *b = config_get_d(CONFIG_JOYSTICK_BUTTON_A);
                *s = state.last_A = state.A;
                ch = 1;
            }
            else if (state.B != state.last_B)
            {
                *b = config_get_d(CONFIG_JOYSTICK_BUTTON_B);
                *s = state.last_B = state.B;
                ch = 1;
            }
            else if (state.P != state.last_P)
            {
                *b = config_get_d(CONFIG_JOYSTICK_BUTTON_R);
                *s = state.last_P = state.P;
                ch = 1;
            }
            else if (state.M != state.last_M)
            {
                *b = config_get_d(CONFIG_JOYSTICK_BUTTON_L);
                *s = state.last_M = state.M;
                ch = 1;
            }
            else if (state.H != state.last_H)
            {
                *b = config_get_d(CONFIG_JOYSTICK_BUTTON_EXIT);
                *s = state.last_H = state.H;
                ch = 1;
            }
        }
        SDL_mutexV(mutex);
    }
    return ch;
}

void tilt_get_direct(int *x, int *y)
{
    *x = 1;
    *y = 1;

    if (mutex)
    {
        SDL_mutexP(mutex);
        {
            if      (state.L)
                *x = -JOY_MAX;
            else if (state.R)
                *x = +JOY_MAX;

            if      (state.U)
                *y = -JOY_MAX;
            else if (state.D)
                *y = +JOY_MAX;
        }
        SDL_mutexV(mutex);
    }
}

float tilt_get_x(void)
{
    float x = 0.0f;

    if (mutex)
    {
        SDL_mutexP(mutex);
        x = state.x;
        SDL_mutexV(mutex);
    }

    return x;
}

float tilt_get_z(void)
{
    float z = 0.0f;

    if (mutex)
    {
        SDL_mutexP(mutex);
        z = state.z;
        SDL_mutexV(mutex);
    }

    return z;
}

int tilt_stat(void)
{
    int b = 0;

    if (mutex)
    {
        SDL_mutexP(mutex);
        b = state.status;
        SDL_mutexV(mutex);
    }
    return b;
}

#endif
/*---------------------------------------------------------------------------*/
#ifndef ENABLE_WII

void tilt_init(void)
{
}

void tilt_free(void)
{
}

int tilt_stat(void)
{
    return 0;
}

int  tilt_get_button(int *b, int *s)
{
    return 0;
}

void tilt_get_direct(int *x, int *y)
{
    *x = 1;
    *y = 1;
}

float tilt_get_x(void)
{
    return 0.0f;
}

float tilt_get_z(void)
{
    return 0.0f;
}

#endif
/*---------------------------------------------------------------------------*/
