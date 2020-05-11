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

#define _ENABLE_TILT
#include "wiiuse.h"

#define NB_WIIMOTES 1
#define NB_WIIMOTE_BUTTONS 15
#define NUNCHUK_CAMERA_LEFT 0
#define NUNCHUK_CAMERA_RIGHT 1

/*---------------------------------------------------------------------------*/

static int wiiUseButtons[NB_WIIMOTE_BUTTONS][2];

struct tilt_state
{
    int status;
    float x;
    float z;
    int buttons[NB_WIIMOTE_BUTTONS];
};

static struct tilt_state current_state, polled_state;
static SDL_mutex        *mutex  = NULL;
static SDL_Thread       *thread = NULL;

static int tilt_thread(void *data)
{
    wiimote** wiimotes = NULL;
    int found = 0;
    int connected = 0;
    struct wiimote_t* wm = NULL;
    int i = 0;
    float pitch_sensitivity = 1.;
    float roll_sensitivity = 1.;
    float smooth_alpha = 0.5;

    wiimotes = wiiuse_init(NB_WIIMOTES);

    found = wiiuse_find(wiimotes, NB_WIIMOTES, 5);
    if (!found) {
        log_printf("No wiimotes found.\n");
        return 0;
    }

    connected = wiiuse_connect(wiimotes, NB_WIIMOTES);
    if (connected) {
        log_printf("Connected to %i wiimotes (of %i found).\n", connected, found);
    } else {
        log_printf("Failed to connect to any wiimote.\n");
        return 0;
    }
    wm = wiimotes[0];

    pitch_sensitivity = ((float)config_get_d(CONFIG_WIIMOTE_PITCH_SENSITIVITY) / 100);
    pitch_sensitivity *= (config_get_d(CONFIG_WIIMOTE_INVERT_PITCH)) ? -1 : 1;
    roll_sensitivity = ((float)config_get_d(CONFIG_WIIMOTE_ROLL_SENSITIVITY) / 100);
    roll_sensitivity *= (config_get_d(CONFIG_WIIMOTE_INVERT_ROLL)) ? -1 : 1;
    smooth_alpha = (float)config_get_d(CONFIG_WIIMOTE_SMOOTH_ALPHA) / 100;

    wiiuse_motion_sensing(wm, 1);
    wiiuse_set_smooth_alpha(wm, smooth_alpha);
    wiiuse_set_leds(wm, WIIMOTE_LED_1);

    SDL_mutexP(mutex);
    current_state.status = 1;
    SDL_mutexV(mutex);

    while (mutex && current_state.status && WIIMOTE_IS_CONNECTED(wm)) {
        if (wiiuse_poll(wiimotes, NB_WIIMOTES)) {
            switch (wm->event) {
                case WIIUSE_EVENT:
                    SDL_mutexP(mutex);
                    /* start on 4 because the 4 first buttons are for the nunchuk */
                    for (i = 4; i < NB_WIIMOTE_BUTTONS; i++) {
                        current_state.buttons[i] = IS_PRESSED(wm, wiiUseButtons[i][0]);
                    }
                    /* if the nunchuk is connected, use it, else use the wiimote */
                    if (wm->exp.type == EXP_NUNCHUK || wm->exp.type == EXP_MOTION_PLUS_NUNCHUK) {
                        struct nunchuk_t* nc = (nunchuk_t*)&wm->exp.nunchuk;
                        current_state.buttons[NUNCHUK_CAMERA_LEFT] = (nc->js.x < -0.3);
                        current_state.buttons[NUNCHUK_CAMERA_RIGHT] = (nc->js.x > 0.3);
                        for (i = 2; i < 4; i++) {
                            current_state.buttons[i] = IS_PRESSED(nc, wiiUseButtons[i][0]);
                        }
                        current_state.x = nc->orient.pitch * pitch_sensitivity;
                        current_state.z = nc->orient.roll * roll_sensitivity;
                    }
                    else if (WIIUSE_USING_ACC(wm)) {
                        if (config_get_d(CONFIG_WIIMOTE_HOLD_SIDEWAYS)) {
                            current_state.x = wm->orient.roll * roll_sensitivity;
                            current_state.z = -wm->orient.pitch * pitch_sensitivity;
                        }
                        else {
                            current_state.x = wm->orient.pitch * pitch_sensitivity;
                            current_state.z = wm->orient.roll * roll_sensitivity;
                        }
                    }
                    SDL_mutexV(mutex);
                    break;
                default:
                    break;
            }
        }
    }

    wiiuse_set_leds(wm, 0);
    wiiuse_cleanup(wiimotes, NB_WIIMOTES);
    return 0;
}

void tilt_init(void)
{
    int wiiUseButtonsHoldNormal[NB_WIIMOTE_BUTTONS][2] = {
        {NUNCHUK_CAMERA_LEFT    , CONFIG_JOYSTICK_BUTTON_L1},
        {NUNCHUK_CAMERA_RIGHT   , CONFIG_JOYSTICK_BUTTON_R1},
        {NUNCHUK_BUTTON_C       , CONFIG_JOYSTICK_BUTTON_X},
        {NUNCHUK_BUTTON_Z       , CONFIG_JOYSTICK_BUTTON_Y},
        {WIIMOTE_BUTTON_A       , CONFIG_JOYSTICK_BUTTON_A},
        {WIIMOTE_BUTTON_B       , CONFIG_JOYSTICK_BUTTON_B},
        {WIIMOTE_BUTTON_MINUS   , CONFIG_JOYSTICK_BUTTON_L1},
        {WIIMOTE_BUTTON_PLUS    , CONFIG_JOYSTICK_BUTTON_R1},
        {WIIMOTE_BUTTON_ONE     , CONFIG_JOYSTICK_BUTTON_X},
        {WIIMOTE_BUTTON_TWO     , CONFIG_JOYSTICK_BUTTON_Y},
        {WIIMOTE_BUTTON_HOME    , CONFIG_JOYSTICK_BUTTON_START},
        {WIIMOTE_BUTTON_LEFT    , CONFIG_JOYSTICK_DPAD_L},
        {WIIMOTE_BUTTON_RIGHT   , CONFIG_JOYSTICK_DPAD_R},
        {WIIMOTE_BUTTON_DOWN    , CONFIG_JOYSTICK_DPAD_D},
        {WIIMOTE_BUTTON_UP      , CONFIG_JOYSTICK_DPAD_U}
    };
    int wiiUseButtonsHoldSideways[NB_WIIMOTE_BUTTONS][2] = {
        {NUNCHUK_CAMERA_LEFT    , CONFIG_JOYSTICK_BUTTON_L1},
        {NUNCHUK_CAMERA_RIGHT   , CONFIG_JOYSTICK_BUTTON_R1},
        {NUNCHUK_BUTTON_C       , CONFIG_JOYSTICK_BUTTON_X},
        {NUNCHUK_BUTTON_Z       , CONFIG_JOYSTICK_BUTTON_Y},
        {WIIMOTE_BUTTON_A       , CONFIG_JOYSTICK_BUTTON_A},
        {WIIMOTE_BUTTON_B       , CONFIG_JOYSTICK_BUTTON_B},
        {WIIMOTE_BUTTON_MINUS   , CONFIG_JOYSTICK_BUTTON_X},
        {WIIMOTE_BUTTON_PLUS    , CONFIG_JOYSTICK_BUTTON_Y},
        {WIIMOTE_BUTTON_ONE     , CONFIG_JOYSTICK_BUTTON_L1},
        {WIIMOTE_BUTTON_TWO     , CONFIG_JOYSTICK_BUTTON_R1},
        {WIIMOTE_BUTTON_HOME    , CONFIG_JOYSTICK_BUTTON_START},
        {WIIMOTE_BUTTON_LEFT    , CONFIG_JOYSTICK_DPAD_D},
        {WIIMOTE_BUTTON_RIGHT   , CONFIG_JOYSTICK_DPAD_U},
        {WIIMOTE_BUTTON_DOWN    , CONFIG_JOYSTICK_DPAD_R},
        {WIIMOTE_BUTTON_UP      , CONFIG_JOYSTICK_DPAD_L}
    };

    memset(&current_state, 0, sizeof (struct tilt_state));
    if  (config_get_d(CONFIG_WIIMOTE_HOLD_SIDEWAYS))
        memcpy(wiiUseButtons, wiiUseButtonsHoldSideways, sizeof(wiiUseButtons));
    else
        memcpy(wiiUseButtons, wiiUseButtonsHoldNormal, sizeof(wiiUseButtons));

    mutex  = SDL_CreateMutex();
    thread = SDL_CreateThread(tilt_thread, "", NULL);
}

void tilt_free(void)
{
    if (mutex)
    {
        /* Get/set the status of the tilt sensor thread. */

        SDL_mutexP(mutex);
        current_state.status = 0;
        SDL_mutexV(mutex);

        /* Wait for the thread to terminate and destroy the mutex. */

        SDL_WaitThread(thread, NULL);
        SDL_DestroyMutex(mutex);
        mutex  = NULL;
        thread = NULL;
    }
}

int tilt_get_button(int *b, int *s)
{
    int i = NB_WIIMOTE_BUTTONS;

    if (mutex) {
        SDL_mutexP(mutex);
        if (current_state.status) {
            for (i = 0; i < NB_WIIMOTE_BUTTONS; i++) {
                if (current_state.buttons[i] != polled_state.buttons[i]) {
                    *b = config_get_d(wiiUseButtons[i][1]);
                    *s = current_state.buttons[i];
                    polled_state.buttons[i] = current_state.buttons[i];
                    break;
                }
            }
        }
        SDL_mutexV(mutex);
    }

    return i < NB_WIIMOTE_BUTTONS;
}

float tilt_get_x(void)
{
    float x = 0.0f;

    if (mutex)
    {
        SDL_mutexP(mutex);
        x = current_state.x;
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
        z = current_state.z;
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
        b = current_state.status;
        SDL_mutexV(mutex);
    }
    return b;
}

/*---------------------------------------------------------------------------*/
