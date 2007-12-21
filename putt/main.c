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

/*---------------------------------------------------------------------------*/

#include <SDL.h>
#include <SDL_image.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "glext.h"
#include "audio.h"
#include "image.h"
#include "state.h"
#include "config.h"
#include "course.h"
#include "hole.h"
#include "game.h"
#include "gui.h"
#include "text.h"

#include "st_conf.h"
#include "st_all.h"

#define TITLE "Neverputt"

/*---------------------------------------------------------------------------*/

static int shot(void)
{
    static char filename[MAXSTR];
    static int  num = 0;

    sprintf(filename, "screen%02d.png", num++);

    image_snap(filename);

    return 1;
}

/*---------------------------------------------------------------------------*/

static void toggle_wire(void)
{
    static int wire = 0;

    if (wire)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        wire = 0;
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        wire = 1;
    }
}
/*---------------------------------------------------------------------------*/

static int loop(void)
{
    SDL_Event e;
    int d = 1;
    int c;

    while (d && SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            return 0;

        switch (e.type)
        {
        case SDL_MOUSEMOTION:
            st_point(+e.motion.x,
                     -e.motion.y + config_get_d(CONFIG_HEIGHT),
                     +e.motion.xrel,
                     -e.motion.yrel);
            break;

        case SDL_MOUSEBUTTONDOWN:
            d = st_click((e.button.button == SDL_BUTTON_LEFT) ? -1 : 1, 1);
            break;

        case SDL_MOUSEBUTTONUP:
            d = st_click((e.button.button == SDL_BUTTON_LEFT) ? -1 : 1, 0);
            break;

        case SDL_KEYDOWN:

            c = e.key.keysym.sym;

            if (config_tst_d(CONFIG_KEY_FORWARD, c))
                st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_Y), -JOY_MAX);

            else if (config_tst_d(CONFIG_KEY_BACKWARD, c))
                st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_Y), +JOY_MAX);

            else if (config_tst_d(CONFIG_KEY_LEFT, c))
                st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_X), -JOY_MAX);

            else if (config_tst_d(CONFIG_KEY_RIGHT, c))
                st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_X), +JOY_MAX);

            else switch (c)
            {
            case SDLK_F10: d = shot();                break;
            case SDLK_F9:  config_tgl_d(CONFIG_FPS);  break;
            case SDLK_F8:  config_tgl_d(CONFIG_NICE); break;
            case SDLK_F7:  toggle_wire();             break;

            case SDLK_RETURN:
                d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 1);
                break;
            case SDLK_ESCAPE:
                d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_EXIT), 1);
                break;

            default:
                d = st_keybd(e.key.keysym.sym, 1);
            }
            break;

        case SDL_KEYUP:

            c = e.key.keysym.sym;

            /* gui_stick needs a non-null value, so we use 1 instead of 0. */

            if (config_tst_d(CONFIG_KEY_FORWARD, c))
                st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_Y), 1);

            else if (config_tst_d(CONFIG_KEY_BACKWARD, c))
                st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_Y), 1);

            else if (config_tst_d(CONFIG_KEY_LEFT, c))
                st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_X), 1);

            else if (config_tst_d(CONFIG_KEY_RIGHT, c))
                st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_X), 1);

            else switch (c)
            {
            case SDLK_RETURN:
                d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 0);
                break;
            case SDLK_ESCAPE:
                d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_EXIT), 0);
                break;

            default:
                d = st_keybd(e.key.keysym.sym, 0);
            }

        case SDL_ACTIVEEVENT:
            if (e.active.state == SDL_APPINPUTFOCUS)
            {
                if (e.active.gain == 0)
                    goto_pause(&st_over, 0);
            }
            break;

        case SDL_JOYAXISMOTION:
            st_stick(e.jaxis.axis, e.jaxis.value);
            break;

        case SDL_JOYBUTTONDOWN:
            d = st_buttn(e.jbutton.button, 1);
            break;

        case SDL_JOYBUTTONUP:
            d = st_buttn(e.jbutton.button, 0);
            break;
        }
    }
    return d;
}

int main(int argc, char *argv[])
{
    int camera = 0;
    SDL_Surface *icon;
    SDL_Joystick *joy = NULL;

    srand((int) time(NULL));

    lang_init("neverball", CONFIG_LOCALE);

    if (config_data_path((argc > 1 ? argv[1] : NULL), COURSE_FILE))
    {
        if (config_user_path(NULL))
        {
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) == 0)
            {
                config_init();
                config_load();

                /* Cache Neverball's camera setting. */

                camera = config_get_d(CONFIG_CAMERA);

                /* Initialize the joystick. */

                if (SDL_NumJoysticks() > 0)
                {
                    joy = SDL_JoystickOpen(config_get_d(CONFIG_JOYSTICK_DEVICE));
                    if (joy)
                    {
                        SDL_JoystickEventState(SDL_ENABLE);
                        set_joystick(joy);
                    }
                }

                /* Initialize the audio. */

                audio_init();

                /* Require 16-bit double buffer with 16-bit depth buffer. */

                SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
                SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
                SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
                SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
                SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#ifndef __APPLE__
                icon = IMG_Load(config_data("icon/neverputt.png"));

                if (icon)
                {
                    SDL_WM_SetIcon(icon, NULL);
                    SDL_FreeSurface(icon);
                }
#endif /* __APPLE__ */

                /* Initialize the video. */

                if (config_mode(config_get_d(CONFIG_FULLSCREEN),
                                config_get_d(CONFIG_WIDTH),
                                config_get_d(CONFIG_HEIGHT)))
                {
                    int t1, t0 = SDL_GetTicks();

                    SDL_WM_SetCaption(TITLE, TITLE);

                    /* Run the main game loop. */

                    init_state(&st_null);
                    goto_state(&st_title);

                    while (loop())
                        if ((t1 = SDL_GetTicks()) > t0)
                        {
                            st_timer((t1 - t0) / 1000.f);
                            st_paint();
                            SDL_GL_SwapBuffers();

                            t0 = t1;

                            if (config_get_d(CONFIG_NICE))
                                SDL_Delay(1);
                        }
                }
                else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());

                /* Restore Neverball's camera setting. */

                config_set_d(CONFIG_CAMERA, camera);
                config_save();

                SDL_Quit();
            }
            else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());
        }
        else fprintf(stderr, L_("Failure to establish config directory\n"));
    }
    else fprintf(stderr, L_("Failure to establish game data directory\n"));

    return 0;
}

/*---------------------------------------------------------------------------*/

