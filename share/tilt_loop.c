/*
 * Copyright (C) 2010 Neverball contributors
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

/* FIXME: I did not copy paste this from tilt_wii.c, I swear! */

/*
 * This data structure tracks button changes, counting transitions so that
 * none are missed if the event handling thread falls significantly behind
 * the device IO thread.
 */

#define BUTTON_NC 0
#define BUTTON_DN 1
#define BUTTON_UP 2

struct button_state
{
    unsigned char curr;
    unsigned char last;
    unsigned char upc;
    unsigned char dnc;
};

static void set_button(struct button_state *B, int s)
{
    if ((B->curr == 0) != (s == 0))
    {
        if (B->curr)
        {
            B->upc++;
            B->curr = 0;
        }
        else
        {
            B->dnc++;
            B->curr = 1;
        }
    }
}

static int get_button(struct button_state *B)
{
    int ch = BUTTON_NC;

    if      (B->last == 1 && B->upc > 0)
    {
        B->upc--;
        B->last = 0;
        ch = BUTTON_UP;
    }
    else if (B->last == 0 && B->dnc > 0)
    {
        B->dnc--;
        B->last = 1;
        ch = BUTTON_DN;
    }

    return ch;
}

/*---------------------------------------------------------------------------*/

struct tilt_state
{
    int   status;
    float x;
    float z;
    struct button_state A;
    struct button_state B;
    struct button_state plus;
    struct button_state minus;
    struct button_state home;
    struct button_state L;
    struct button_state R;
    struct button_state U;
    struct button_state D;
};

static struct tilt_state state;
static SDL_mutex        *mutex  = NULL;
static SDL_Thread       *thread = NULL;

/*---------------------------------------------------------------------------*/

#include "freespace/freespace.h"
#include "freespace/freespace_codecs.h"
#include "vec3.h"

#define FILTER 8
#define DAMPENING 1

static int tilt_func(void *data)
{
    FreespaceDeviceId deviceId;
    uint8_t buffer[FREESPACE_MAX_OUTPUT_MESSAGE_SIZE];
    int rc;
    float x, y, z;
    struct freespace_DataMotionControl d;
    struct freespace_UserFrame userFrame;
    int running = 1;
    int length;
    int numIds;

    float quat[4];
    float eulerAngles[3];


    rc = freespace_getDeviceList(&deviceId, 1, &numIds);
    if (numIds == 0) {
        return 1;
    }

    rc = freespace_openDevice(deviceId);
    if (rc != FREESPACE_SUCCESS) {
        return 1;
    }

    rc = freespace_flush(deviceId);
    if (rc != FREESPACE_SUCCESS) {
        return 1;
    }

    d.enableBodyMotion = 0;
    d.enableUserPosition = 1;
    d.inhibitPowerManager = 0;
    d.enableMouseMovement = 1;
    d.disableFreespace = 0;
    rc = freespace_encodeDataMotionControl(&d, buffer, sizeof(buffer));
    if (rc > 0) {
        rc = freespace_send(deviceId, buffer, rc);
        if (rc != FREESPACE_SUCCESS) {
            return 1;
        }
    }


    SDL_mutexP(mutex);
    state.status = running;
    SDL_mutexV(mutex);

    while (mutex && running)
    {
        SDL_mutexP(mutex);
        running = state.status;
        SDL_mutexV(mutex);

        rc = freespace_read(deviceId, buffer, FREESPACE_MAX_INPUT_MESSAGE_SIZE, 100, &length);
        if (rc != FREESPACE_SUCCESS) {
            continue;
        }

        if (length == 0) {
            continue;
        }

        if (freespace_decodeUserFrame(buffer, length, &userFrame) == FREESPACE_SUCCESS) {
            /* Hillcrest quaternion is rotate the world type, so make it rotate the object type by conjugating*/
            quat[0] = userFrame.angularPosA;
            quat[1] = -userFrame.angularPosB;
            quat[2] = -userFrame.angularPosC;
            quat[3] = -userFrame.angularPosD;
            q_nrm(quat, quat);

            /* This function does euler decomposition for rotate the object type (ZYX, aerospace) */
            q_euler(eulerAngles, quat);

            SDL_mutexP(mutex);
            {
                /* Since the game expects "rotate the world type", conjugate by negating all angles & convert to degrees
                 * Z is yaw
                 * Y is pitch
                 * X is roll */
                z = -eulerAngles[0] * 57.2957795;
                y = -eulerAngles[1] * 57.2957795;
                x = -eulerAngles[2] * 57.2957795;

                state.x = y * DAMPENING;
                state.z = -x * DAMPENING;

                set_button(&state.home, userFrame.button3);
                set_button(&state.U, userFrame.deltaWheel > 0);
                set_button(&state.D, userFrame.deltaWheel < 0);
            }
            SDL_mutexV(mutex);
        }

    }

    freespace_closeDevice(deviceId);
    return 0;
}

void tilt_init(void)
{
    memset(&state, 0, sizeof (struct tilt_state));

    freespace_init();

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

        SDL_WaitThread(thread, &b);
        SDL_DestroyMutex(mutex);

        mutex  = NULL;
        thread = NULL;

        freespace_exit();
    }
}

int tilt_get_button(int *b, int *s)
{
    int ch = BUTTON_NC;

    if (mutex)
    {
        SDL_mutexP(mutex);
        {
            if      ((ch = get_button(&state.A)))
            {
                *b = config_get_d(CONFIG_JOYSTICK_BUTTON_A);
                *s = (ch == BUTTON_DN);
            }
            else if ((ch = get_button(&state.B)))
            {
                *b = config_get_d(CONFIG_JOYSTICK_BUTTON_B);
                *s = (ch == BUTTON_DN);
            }
            else if ((ch = get_button(&state.home)))
            {
                *b = config_get_d(CONFIG_JOYSTICK_BUTTON_EXIT);
                *s = (ch == BUTTON_DN);
            }
            else if ((ch = get_button(&state.U)))
            {
                *b = config_get_d(CONFIG_JOYSTICK_DPAD_U);
                *s = (ch == BUTTON_DN);
            }
            else if ((ch = get_button(&state.D)))
            {
                *b = config_get_d(CONFIG_JOYSTICK_DPAD_D);
                *s = (ch == BUTTON_DN);
            }
        }
        SDL_mutexV(mutex);
    }
    return ch;
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

/*---------------------------------------------------------------------------*/
