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
#include "image.h"
#include "state.h"
#include "config.h"

#define TITLE "Neverball"

/*---------------------------------------------------------------------------*/

static int shot(void)
{
    static char filename[STRMAX];
    static int  num = 0;

    sprintf(filename, "screen%02d.bmp", num++);

    image_snap(filename);

    return 1;
}

static int demo(void)
{
    static char file[STRMAX];
    static char src[STRMAX];
    static char dst[STRMAX];
    static int  num = 0;

    sprintf(file, "demo%02d.dat", num++);

    /* Make sure the recording file is closed before renaming it. */

    if (record_fp)
    {
        fclose(record_fp);
        record_fp = NULL;
    }

    if (config_home(dst, file, STRMAX) &&
        config_home(src, USER_REPLAY_FILE, STRMAX))
        rename(src, dst);

    return 1;
}

/*---------------------------------------------------------------------------*/

static int grabbed = 0;

static void toggle_grab(void)
{
    grabbed = 1 - grabbed;

    if (grabbed)
    {
        SDL_WM_GrabInput(SDL_GRAB_ON);
        Mix_ResumeMusic();
    }
    else
    {
        SDL_WM_GrabInput(SDL_GRAB_OFF);
        Mix_PauseMusic();
    }
}

static void darken()
{
    int w = config_w();
    int h = config_h();

    config_push_ortho();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    {
        glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
        glVertex2i(0, 0);
        glVertex2i(w, 0);
        glVertex2i(w, h);
        glVertex2i(0, h);
    }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    config_pop_matrix();
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
                d = st_point(+e.motion.x,
                             -e.motion.y + config_h(),
                             +e.motion.xrel,
                             config_inv() ? e.motion.yrel : -e.motion.yrel);
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
            if (e.key.keysym.sym == SDLK_SPACE) { toggle_grab();      break; }
            if (e.key.keysym.sym == SDLK_F11)   { d = demo();         break; }
            if (e.key.keysym.sym == SDLK_F10)   { d = shot();         break; }
            if (e.key.keysym.sym == SDLK_F9)    { config_tog_fps();   break; }
            if (e.key.keysym.sym == SDLK_F8)    { config_tog_nice();  break; }
            if (e.key.keysym.sym == SDLK_F1)    { config_set_view(0); break; }
            if (e.key.keysym.sym == SDLK_F2)    { config_set_view(1); break; }
            if (e.key.keysym.sym == SDLK_F3)    { config_set_view(2); break; }
            
            if (grabbed)
                d = st_keybd(e.key.keysym.sym);
            break;

        case SDL_JOYAXISMOTION:
            if (grabbed)
                d = st_stick(e.jaxis.axis, e.jaxis.value);
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
    const char  *path = (argc > 1) ? argv[1] : CONFIG_PATH;
    SDL_Joystick *joy = NULL;

    if (config_path(path))
    {
        config_load();

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) == 0)
        {
            /* Initialize the joystick. */

            if (SDL_NumJoysticks() > 0)
            {
                if ((joy = SDL_JoystickOpen(config_joy_device())))
                    SDL_JoystickEventState(SDL_ENABLE);
            }

            /* Initialize the audio. */

            if (config_set_audio(config_rate(), config_buff()))
            {
                /* Require 16-bit double buffer with 16-bit depth buffer. */

                SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
                SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
                SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
                SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
                SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
                SDL_GL_SetAttribute(SDL_GL_STEREO, config_stereo() ? 1 : 0);

                /* Initialize the video. */

                if (config_set_mode(config_w(), config_h(), config_mode()))
                {
                    int t1, t0 = SDL_GetTicks();

                    SDL_WM_SetCaption(TITLE, TITLE); 
                    SDL_ShowCursor(SDL_DISABLE);
                    toggle_grab();

                    /* Run the main game loop. */

                    init_state();

                    while (loop())
                        if ((t1 = SDL_GetTicks()) > t0)
                        {
                            if (grabbed)
                                st_timer((t1 - t0) / 1000.0);

                            st_paint();

                            if (!grabbed)
                                darken();

                            SDL_GL_SwapBuffers();

                            t0 = t1;

                            if (config_nice())
                                SDL_Delay(1);
                        }

                    SDL_ShowCursor(SDL_ENABLE);
                }
                else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());
            }
            else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());

            if (SDL_JoystickOpened(0))
                SDL_JoystickClose(joy);

            SDL_Quit();
        }
        else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());

        config_store();
    }
    else fprintf(stderr, "%s: Can't find data directory\n", argv[0]);

    return 0;
}

/*---------------------------------------------------------------------------*/

