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

/*---------------------------------------------------------------------------*/

#ifdef WIN32
#pragma comment(lib, "SDL_ttf.lib")
#pragma comment(lib, "SDL_mixer.lib")
#pragma comment(lib, "SDL_image.lib")
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "opengl32.lib")
#endif

/*---------------------------------------------------------------------------*/

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string.h>

#include "glext.h"
#include "config.h"
#include "image.h"
#include "audio.h"
#include "demo.h"
#include "gui.h"
#include "set.h"

#include "st_conf.h"
#include "st_title.h"

#define TITLE "Neverball"

/*---------------------------------------------------------------------------*/

static void shot(void)
{
    static char filename[MAXSTR];
    static int  num = 0;

    sprintf(filename, "screen%02d.bmp", num++);

    image_snap(filename);
}

static void demo(void)
{
    static char file[MAXSTR];
    static char src[MAXSTR];
    static char dst[MAXSTR];
    static int  num = 0;

    sprintf(file, "demo%02d.dat", num++);

    demo_finish();

    if (config_home(dst, file, MAXSTR) &&
        config_home(src, USER_REPLAY_FILE, MAXSTR))
        rename(src, dst);
}

/*---------------------------------------------------------------------------*/

static int grabbed = 0;

static void enable_grab(void)
{
    if (grabbed != 1)
    {
        SDL_WM_GrabInput(SDL_GRAB_ON);
        Mix_ResumeMusic();
        grabbed = 1;
    }
}

static void disable_grab(void)
{
    if (grabbed != 0)
    {
        grabbed = 0;
        Mix_PauseMusic();
        SDL_WM_GrabInput(SDL_GRAB_OFF);
    }
}

static void toggle_grab(void)
{
    if (grabbed)
        disable_grab();
    else
        enable_grab();
}

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

    while (d && SDL_PollEvent(&e))
        switch (e.type)
        {
        case SDL_MOUSEMOTION:
            if (grabbed)
                st_point(+e.motion.x,
                         -e.motion.y + config_get(CONFIG_HEIGHT),
                         +e.motion.xrel,
                         config_get(CONFIG_MOUSE_INVERT)
                         ? +e.motion.yrel
                         : -e.motion.yrel);
              break;

        case SDL_MOUSEBUTTONDOWN:
            if (grabbed)
                d = st_click((e.button.button == SDL_BUTTON_LEFT) ? -1 : 1, 1);
            break;

        case SDL_MOUSEBUTTONUP:
            if (grabbed)
                d = st_click((e.button.button == SDL_BUTTON_LEFT) ? -1 : 1, 0);
            break;

        case SDL_KEYDOWN:

            if (e.key.keysym.sym == SDLK_SPACE)
                toggle_grab();

            if (grabbed)
                switch (e.key.keysym.sym)
                {
                case SDLK_F11:   demo();                  break;
                case SDLK_F10:   shot();                  break;
                case SDLK_F9:    config_tgl(CONFIG_FPS);  break;
                case SDLK_F8:    config_tgl(CONFIG_NICE); break;
                case SDLK_F7:    toggle_wire();           break;

                case SDLK_RETURN:
                    d = st_buttn(config_get(CONFIG_JOYSTICK_BUTTON_A), 1);
                    break;
                case SDLK_LEFT:
                    st_stick(config_get(CONFIG_JOYSTICK_AXIS_X), -JOY_MAX);
                    break;
                case SDLK_RIGHT:
                    st_stick(config_get(CONFIG_JOYSTICK_AXIS_X), +JOY_MAX);
                    break;
                case SDLK_UP:
                    st_stick(config_get(CONFIG_JOYSTICK_AXIS_Y), -JOY_MAX);
                    break;
                case SDLK_DOWN:
                    st_stick(config_get(CONFIG_JOYSTICK_AXIS_Y), +JOY_MAX);
                    break;
                             
                default: 
                    if (grabbed)
                        d = st_keybd(e.key.keysym.sym, 1);
                }

            break;

        case SDL_KEYUP:

            if (grabbed)
                switch (e.key.keysym.sym)
                {
                case SDLK_RETURN:
                    d = st_buttn(config_get(CONFIG_JOYSTICK_BUTTON_A), 0);
                    break;
                case SDLK_LEFT:
                case SDLK_RIGHT:
                    st_stick(config_get(CONFIG_JOYSTICK_AXIS_X), 0);
                    break;
                case SDLK_DOWN:
                case SDLK_UP:
                    st_stick(config_get(CONFIG_JOYSTICK_AXIS_Y), 0);
                    break;

                default:
                    if (grabbed)
                        d = st_keybd(e.key.keysym.sym, 0);
                }

            break;

        case SDL_ACTIVEEVENT:
            if (e.active.state == SDL_APPINPUTFOCUS && e.active.gain == 0)
                disable_grab();
            break;

        case SDL_JOYAXISMOTION:
            if (grabbed)
                st_stick(e.jaxis.axis, e.jaxis.value);
            break;

        case SDL_JOYBUTTONDOWN:
            if (grabbed)
                d = st_buttn(e.jbutton.button, 1);
            break;

        case SDL_JOYBUTTONUP:
            if (grabbed)
                d = st_buttn(e.jbutton.button, 0);
            break;

        case SDL_QUIT:
            d = 0;
        }

    return d;
}

int main(int argc, char *argv[])
{
    char  *path = (argc > 1) ? argv[1] : CONFIG_PATH;
    SDL_Joystick *joy = NULL;

    if (config_path(path, SET_FILE))
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) == 0)
        {
            config_init();
            config_load();

            /* Initialize the joystick. */

            if (SDL_NumJoysticks() > 0)
            {
                joy = SDL_JoystickOpen(config_get(CONFIG_JOYSTICK_DEVICE));

                if (joy)
                    SDL_JoystickEventState(SDL_ENABLE);
            }

            /* Initialize the audio. */

            audio_init();

            /* Require 16-bit double buffer with 16-bit depth buffer. */

            SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

            /* Initialize the video. */

            if (config_mode(config_get(CONFIG_FULLSCREEN),
                            config_get(CONFIG_WIDTH),
                            config_get(CONFIG_HEIGHT)))
            {
                int t1, t0 = SDL_GetTicks();

                SDL_WM_SetCaption(TITLE, TITLE); 
                enable_grab();

                /* Initialize the run state and the title display. */

                init_state(&st_null);
                goto_state(&st_title);

                /* Run the main game loop. */

                while (loop())
                    if ((t1 = SDL_GetTicks()) > t0)
                    {
                        if (grabbed)
                            st_timer((t1 - t0) / 1000.f);

                        st_paint();

                        if (!grabbed)
                            gui_blank();

                        SDL_GL_SwapBuffers();

                        t0 = t1;

                        if (config_get(CONFIG_NICE))
                            SDL_Delay(1);
                    }
            }
            else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());

            if (SDL_JoystickOpened(0))
                SDL_JoystickClose(joy);

            SDL_Quit();
        }
        else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());

        config_save();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

