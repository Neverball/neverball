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
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glext.h"
#include "audio.h"
#include "image.h"
#include "state.h"
#include "config.h"
#include "hole.h"

#define TITLE "Neverputt"

/*---------------------------------------------------------------------------*/

static int shot(void)
{
    static char filename[MAXSTR];
    static int  num = 0;

    sprintf(filename, "screen%02d.bmp", num++);

    image_snap(filename);

    return 1;
}

/*---------------------------------------------------------------------------*/

static int grabbed = 0;

static void enable_grab(void)
{
    SDL_WM_GrabInput(SDL_GRAB_ON);
    Mix_ResumeMusic();
    grabbed = 1;
}

static void disable_grab(void)
{
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    Mix_PauseMusic();
    grabbed = 0;
}

static void toggle_grab(void)
{
    if (grabbed)
        disable_grab();
    else
        enable_grab();
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
                             -e.motion.y + config_get(CONFIG_HEIGHT),
                             +e.motion.xrel,
                             -e.motion.yrel);
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
            if (e.key.keysym.sym == SDLK_F10)   { d = shot();         break; }
            if (e.key.keysym.sym == SDLK_F9)    { config_tgl(CONFIG_FPS);   break; }
            if (e.key.keysym.sym == SDLK_F8)    { config_tgl(CONFIG_NICE);  break; }
            
            if (grabbed)
                d = st_keybd(e.key.keysym.sym);
            break;

        case SDL_ACTIVEEVENT:
            if (e.active.state == SDL_APPINPUTFOCUS && e.active.gain == 0)
                disable_grab();
            break;

        case SDL_QUIT:
            d = 0;
        }

    return d;
}

int main(int argc, char *argv[])
{
    char *path = (argc > 1) ? argv[1] : CONFIG_PATH;

    if (config_path(path, HOLE_FILE))
    {
        config_load();

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0)
        {
            /* Initialize the audio. */

            audio_init();

            /* Require 16-bit double buffer with 16-bit depth buffer. */

            SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

            /* Initialize the video. */

            if (config_mode(config_get(CONFIG_FULLSCREEN), config_get(CONFIG_WIDTH), config_get(CONFIG_HEIGHT)))
            {
                int t1, t0 = SDL_GetTicks();

                SDL_WM_SetCaption(TITLE, TITLE); 
                toggle_grab();

                /* Run the main game loop. */

                goto_state(&st_title);

                while (loop())
                    if ((t1 = SDL_GetTicks()) > t0)
                    {
                        if (grabbed)
                            st_timer((t1 - t0) / 1000.f);

                        st_paint();
                        SDL_GL_SwapBuffers();

                        t0 = t1;

                        if (config_get(CONFIG_NICE))
                            SDL_Delay(1);
                    }
            }
            else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());

            SDL_Quit();
        }
        else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());

        config_save();
    }
    else fprintf(stderr, "%s: Can't find data directory\n", argv[0]);


    return 0;
}

/*---------------------------------------------------------------------------*/

